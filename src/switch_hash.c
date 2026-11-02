/*
 * kit : Useful R Functions Implemented in C
 * Copyright (C) 2020-2025  Morgan Jacob
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kit.h"

/*
 * Value -> last pair index map for vswitch()/nswitch() when there are many
 * values. The value-major fallback compares x against every value; this map
 * finds the pair in one probe and then scans x once.
 *
 * Matching follows the fallback exactly: integer/logical use bit equality (so
 * NA matches NA), doubles and complex canonicalise -0 and never match NA/NaN,
 * and strings compare CHARSXP identity (so NA_STRING matches itself).
 *
 * x is processed in blocks so the temporary match indices stay in cache and a
 * long input does not need a second full-length allocation.
 */

#define KIT_SW_BLOCK 4096

static inline uint32_t kit_sw_mix32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x9E3779B1U;
  x ^= x >> 13;
  return x;
}

static inline uint32_t kit_sw_mix64(uint64_t x) {
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  return (uint32_t)x;
}

static inline uint64_t kit_sw_bits_dbl(double x) {
  union { double d; uint64_t u; } v;
  v.d = (x == 0.0) ? 0.0 : x; // -0 == 0
  return v.u;
}

static inline uint32_t kit_sw_hash_int(int x) {
  return kit_sw_mix32((uint32_t)x);
}

static inline uint32_t kit_sw_hash_dbl(double x) {
  return kit_sw_mix64(kit_sw_bits_dbl(x));
}

static inline uint32_t kit_sw_hash_cplx(Rcomplex x) {
  return kit_sw_mix64(kit_sw_bits_dbl(x.r) ^
                      (kit_sw_bits_dbl(x.i) * 0x9E3779B97F4A7C15ULL));
}

static inline uint32_t kit_sw_hash_str(SEXP x) {
  const uintptr_t p = (uintptr_t)x;
  return kit_sw_mix64((uint64_t)p ^ ((uint64_t)p >> 17));
}

static inline bool kit_sw_eq_int(int a, int b) { return a == b; }
static inline bool kit_sw_eq_dbl(double a, double b) {
  return kit_sw_bits_dbl(a) == kit_sw_bits_dbl(b);
}
static inline bool kit_sw_eq_cplx(Rcomplex a, Rcomplex b) {
  return kit_sw_bits_dbl(a.r) == kit_sw_bits_dbl(b.r) &&
         kit_sw_bits_dbl(a.i) == kit_sw_bits_dbl(b.i);
}
static inline bool kit_sw_eq_str(SEXP a, SEXP b) { return a == b; }

static inline bool kit_sw_skip_none_int(int x) { (void)x; return false; }
static inline bool kit_sw_skip_none_str(SEXP x) { (void)x; return false; }
static inline bool kit_sw_skip_dbl(double x) { return ISNAN(x); }
static inline bool kit_sw_skip_cplx(Rcomplex x) { return ISNAN(x.r) || ISNAN(x.i); }

#define KIT_SW_MAP_DEF(TAG, TYPE, HASHFN, EQFN, SKIPFN) \
static inline void kit_sw_build_##TAG(const TYPE *v, const R_xlen_t m, \
                                      int *h, const size_t mask) { \
  for (R_xlen_t i = 0; i < m; ++i) { \
    if (SKIPFN(v[i])) continue; \
    size_t id = (size_t)HASHFN(v[i]) & mask; \
    while (h[id] && !EQFN(v[h[id]-1], v[i])) id = (id + 1) & mask; \
    h[id] = (int)i + 1; /* later pairs overwrite earlier ones */ \
  } \
} \
static inline int kit_sw_get_##TAG(const TYPE *v, const int *h, \
                                   const size_t mask, const TYPE x) { \
  if (SKIPFN(x)) return -1; \
  size_t id = (size_t)HASHFN(x) & mask; \
  while (h[id]) { \
    if (EQFN(v[h[id]-1], x)) return h[id]-1; \
    id = (id + 1) & mask; \
  } \
  return -1; \
}

KIT_SW_MAP_DEF(lgl, int, kit_sw_hash_int, kit_sw_eq_int, kit_sw_skip_none_int)
KIT_SW_MAP_DEF(int, int, kit_sw_hash_int, kit_sw_eq_int, kit_sw_skip_none_int)
KIT_SW_MAP_DEF(dbl, double, kit_sw_hash_dbl, kit_sw_eq_dbl, kit_sw_skip_dbl)
KIT_SW_MAP_DEF(cplx, Rcomplex, kit_sw_hash_cplx, kit_sw_eq_cplx, kit_sw_skip_cplx)
KIT_SW_MAP_DEF(str, SEXP, kit_sw_hash_str, kit_sw_eq_str, kit_sw_skip_none_str)

#define KIT_SW_MATCH_DEF(TAG, TYPE) \
static inline void kit_sw_match_##TAG(const TYPE *px, const TYPE *pv, \
                                      const int *h, const size_t mask, \
                                      int *match, const R_xlen_t off, \
                                      const R_xlen_t len) { \
  for (ssize_t j = 0; j < len; ++j) \
    match[j] = kit_sw_get_##TAG(pv, h, mask, px[off + j]); \
}

KIT_SW_MATCH_DEF(lgl, int)
KIT_SW_MATCH_DEF(int, int)
KIT_SW_MATCH_DEF(dbl, double)
KIT_SW_MATCH_DEF(cplx, Rcomplex)
KIT_SW_MATCH_DEF(str, SEXP)

static inline void kit_sw_write_lgl(SEXP ans, const int *match,
                                    const R_xlen_t off, const R_xlen_t len,
                                    const int *pna, const ssize_t namask,
                                    int *const *ppo, const ssize_t *amask) {
  int *restrict pans = LOGICAL(ans);
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    pans[off + j] = k < 0 ? (pna ? pna[(off + j) & namask] : NA_LOGICAL)
                          : ppo[k][(off + j) & amask[k]];
  }
}

static inline void kit_sw_write_int(SEXP ans, const int *match,
                                    const R_xlen_t off, const R_xlen_t len,
                                    const int *pna, const ssize_t namask,
                                    int *const *ppo, const ssize_t *amask) {
  int *restrict pans = INTEGER(ans);
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    pans[off + j] = k < 0 ? (pna ? pna[(off + j) & namask] : NA_INTEGER)
                          : ppo[k][(off + j) & amask[k]];
  }
}

static inline void kit_sw_write_dbl(SEXP ans, const int *match,
                                    const R_xlen_t off, const R_xlen_t len,
                                    const double *pna, const ssize_t namask,
                                    double *const *ppo, const ssize_t *amask) {
  double *restrict pans = REAL(ans);
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    pans[off + j] = k < 0 ? (pna ? pna[(off + j) & namask] : NA_REAL)
                          : ppo[k][(off + j) & amask[k]];
  }
}

static inline void kit_sw_write_cplx(SEXP ans, const int *match,
                                     const R_xlen_t off, const R_xlen_t len,
                                     const Rcomplex *pna, const ssize_t namask,
                                     Rcomplex *const *ppo,
                                     const ssize_t *amask) {
  Rcomplex *restrict pans = COMPLEX(ans);
  Rcomplex na_cplx;
  na_cplx.r = NA_REAL;
  na_cplx.i = NA_REAL;
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    pans[off + j] = k < 0 ? (pna ? pna[(off + j) & namask] : na_cplx)
                          : ppo[k][(off + j) & amask[k]];
  }
}

static inline void kit_sw_write_str(SEXP ans, const int *match,
                                    const R_xlen_t off, const R_xlen_t len,
                                    const SEXP *pna, const ssize_t namask,
                                    const SEXP *const *ppo,
                                    const ssize_t *amask) {
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    SET_STRING_ELT(ans, off + j,
                   k < 0 ? (pna ? pna[(off + j) & namask] : NA_STRING)
                         : ppo[k][(off + j) & amask[k]]);
  }
}

static inline void kit_sw_write_vec(SEXP ans, const int *match,
                                    const R_xlen_t off, const R_xlen_t len,
                                    const SEXP *pna, const ssize_t namask,
                                    const SEXP *const *ppo,
                                    const ssize_t *amask) {
  for (ssize_t j = 0; j < len; ++j) {
    const int k = match[j];
    if (k < 0) {
      if (pna) SET_VECTOR_ELT(ans, off + j, pna[(off + j) & namask]);
    } else {
      SET_VECTOR_ELT(ans, off + j, ppo[k][(off + j) & amask[k]]);
    }
  }
}

void kit_switch_hash(SEXP ans, SEXP x, SEXP values, SEXP outputs, SEXP na,
                     const SEXPTYPE type_x, const SEXPTYPE type_o,
                     const R_xlen_t len_x, const R_xlen_t len_values,
                     const R_xlen_t len_na, const int nth, const bool nonna,
                     const bool utfcon, SEXP xans, SEXP vans) {
  const bool list_out = type_o == VECSXP;
  const SEXP *po = list_out ? SEXPPTR_RO(outputs) : NULL;
  const SEXPTYPE type_ot = list_out ? UTYPEOF(po[0]) : type_o;
  const ssize_t namask = len_na > 1 ? SSIZE_MAX : 0;
  const SEXP sx = utfcon ? xans : x;
  const SEXP sv = utfcon ? vans : values;
  const int nth_hash = nth < 1 ? 1 : nth;
  const bool serial_out = type_ot == STRSXP || type_ot == VECSXP;

  /* Keep small tables sparse (they still fit in cache) so a miss usually
     stops after one probe; large tables stay at a 2x load factor. */
  size_t cap = 256;
  const size_t target = (size_t)len_values * (len_values <= 4096 ? 4 : 2);
  while (cap < target) cap <<= 1;
  const size_t mask = cap - 1;
  int *slots = (int*) R_Calloc(cap, int);
  int *match = (int*) R_alloc((size_t) nth_hash * KIT_SW_BLOCK, sizeof(int));

  switch (type_x) {
  case LGLSXP:
    kit_sw_build_lgl(LOGICAL(sv), len_values, slots, mask);
    break;
  case INTSXP:
    kit_sw_build_int(INTEGER(sv), len_values, slots, mask);
    break;
  case REALSXP:
    kit_sw_build_dbl(REAL(sv), len_values, slots, mask);
    break;
  case CPLXSXP:
    kit_sw_build_cplx(COMPLEX(sv), len_values, slots, mask);
    break;
  case STRSXP:
    kit_sw_build_str(STRING_PTR_RO(sv), len_values, slots, mask);
    break;
  default: // # nocov
    R_Free(slots); // # nocov
    error("Type %s is not supported for hashed switch.", type2char(type_x)); // # nocov
  }

  /* Output pointers: a list of per-pair outputs, or a vector of pair results. */
  void *ppo_store = NULL;
  ssize_t *amask = (ssize_t*) R_alloc((size_t) len_values, sizeof(ssize_t));
  const SEXP *ppo_vec = NULL;
  const int *ppo_int = NULL;
  const double *ppo_dbl = NULL;
  const Rcomplex *ppo_cplx = NULL;
  const SEXP *ppo_str = NULL;
  switch (type_ot) {
  case LGLSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(int*));
    if (list_out) {
      for (ssize_t i = 0; i < len_values; ++i) {
        ((int**) ppo_store)[i] = LOGICAL(po[i]);
        amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
      }
    } else {
      ppo_int = LOGICAL(outputs);
      for (ssize_t i = 0; i < len_values; ++i) {
        ((int**) ppo_store)[i] = (int*) ppo_int + i;
        amask[i] = 0;
      }
    }
    break;
  case INTSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(int*));
    if (list_out) {
      for (ssize_t i = 0; i < len_values; ++i) {
        ((int**) ppo_store)[i] = INTEGER(po[i]);
        amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
      }
    } else {
      ppo_int = INTEGER(outputs);
      for (ssize_t i = 0; i < len_values; ++i) {
        ((int**) ppo_store)[i] = (int*) ppo_int + i;
        amask[i] = 0;
      }
    }
    break;
  case REALSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(double*));
    if (list_out) {
      for (ssize_t i = 0; i < len_values; ++i) {
        ((double**) ppo_store)[i] = REAL(po[i]);
        amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
      }
    } else {
      ppo_dbl = REAL(outputs);
      for (ssize_t i = 0; i < len_values; ++i) {
        ((double**) ppo_store)[i] = (double*) ppo_dbl + i;
        amask[i] = 0;
      }
    }
    break;
  case CPLXSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(Rcomplex*));
    if (list_out) {
      for (ssize_t i = 0; i < len_values; ++i) {
        ((Rcomplex**) ppo_store)[i] = COMPLEX(po[i]);
        amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
      }
    } else {
      ppo_cplx = COMPLEX(outputs);
      for (ssize_t i = 0; i < len_values; ++i) {
        ((Rcomplex**) ppo_store)[i] = (Rcomplex*) ppo_cplx + i;
        amask[i] = 0;
      }
    }
    break;
  case STRSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(SEXP*));
    if (list_out) {
      for (ssize_t i = 0; i < len_values; ++i) {
        ((const SEXP**) ppo_store)[i] = STRING_PTR_RO(po[i]);
        amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
      }
    } else {
      ppo_str = STRING_PTR_RO(outputs);
      for (ssize_t i = 0; i < len_values; ++i) {
        ((const SEXP**) ppo_store)[i] = (const SEXP*) ppo_str + i;
        amask[i] = 0;
      }
    }
    break;
  case VECSXP:
    ppo_store = R_alloc((size_t) len_values, sizeof(SEXP*));
    for (ssize_t i = 0; i < len_values; ++i) {
      ((const SEXP**) ppo_store)[i] = SEXPPTR_RO(po[i]);
      amask[i] = xlength(po[i]) > 1 ? SSIZE_MAX : 0;
    }
    break;
  default: // # nocov
    R_Free(slots); // # nocov
    error("Type %s is not supported for hashed switch output.", // # nocov
          type2char(type_ot)); // # nocov
  }

  const R_xlen_t nblocks = (len_x + KIT_SW_BLOCK - 1) / KIT_SW_BLOCK;
  #define KIT_SW_MATCH_BLOCK() do { \
    switch (type_x) { \
    case LGLSXP: kit_sw_match_lgl(LOGICAL(sx), LOGICAL(sv), slots, mask, mb, off, blen); break; \
    case INTSXP: kit_sw_match_int(INTEGER(sx), INTEGER(sv), slots, mask, mb, off, blen); break; \
    case REALSXP: kit_sw_match_dbl(REAL(sx), REAL(sv), slots, mask, mb, off, blen); break; \
    case CPLXSXP: kit_sw_match_cplx(COMPLEX(sx), COMPLEX(sv), slots, mask, mb, off, blen); break; \
    case STRSXP: kit_sw_match_str(STRING_PTR_RO(sx), STRING_PTR_RO(sv), slots, mask, mb, off, blen); break; \
    } \
  } while (0)

  #define KIT_SW_WRITE_BLOCK() do { \
    switch (type_ot) { \
    case LGLSXP: kit_sw_write_lgl(ans, mb, off, blen, nonna ? LOGICAL(na) : NULL, namask, (int*const*) ppo_store, amask); break; \
    case INTSXP: kit_sw_write_int(ans, mb, off, blen, nonna ? INTEGER(na) : NULL, namask, (int*const*) ppo_store, amask); break; \
    case REALSXP: kit_sw_write_dbl(ans, mb, off, blen, nonna ? REAL(na) : NULL, namask, (double*const*) ppo_store, amask); break; \
    case CPLXSXP: kit_sw_write_cplx(ans, mb, off, blen, nonna ? COMPLEX(na) : NULL, namask, (Rcomplex*const*) ppo_store, amask); break; \
    case STRSXP: kit_sw_write_str(ans, mb, off, blen, nonna ? STRING_PTR_RO(na) : NULL, namask, (const SEXP*const*) ppo_store, amask); break; \
    case VECSXP: kit_sw_write_vec(ans, mb, off, blen, nonna ? SEXPPTR_RO(na) : NULL, namask, (const SEXP*const*) ppo_store, amask); break; \
    } \
  } while (0)

  if (serial_out) {
    for (R_xlen_t b = 0; b < nblocks; ++b) {
      const R_xlen_t off = b * KIT_SW_BLOCK;
      const R_xlen_t blen = (len_x - off < KIT_SW_BLOCK) ? len_x - off : KIT_SW_BLOCK;
      int *mb = match;
      KIT_SW_MATCH_BLOCK();
      KIT_SW_WRITE_BLOCK();
    }
  } else {
    OMP_PARALLEL_FOR(nth_hash)
    for (R_xlen_t b = 0; b < nblocks; ++b) {
      const R_xlen_t off = b * KIT_SW_BLOCK;
      const R_xlen_t blen = (len_x - off < KIT_SW_BLOCK) ? len_x - off : KIT_SW_BLOCK;
      int *mb = match + (size_t) omp_get_thread_num() * KIT_SW_BLOCK;
      KIT_SW_MATCH_BLOCK();
      KIT_SW_WRITE_BLOCK();
    }
  }
  #undef KIT_SW_MATCH_BLOCK
  #undef KIT_SW_WRITE_BLOCK
  R_Free(slots);
}
