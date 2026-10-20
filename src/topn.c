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
 * Bounded top-n selection for long numeric vectors (> INT_MAX elements) (#83).
 *
 * The previous radix order() path allocated a full-length index vector
 * (8 * length bytes, ~17 GB for 2^31 elements) only to return n indices,
 * and base::order(..., decreasing = TRUE) errors on long vectors. The
 * selection below keeps instead a heap of the n best elements: memory is
 * O(n) (one (value, index) entry plus the returned index, per request).
 *
 * The output matches order(vec, decreasing = dcr, na.last = TRUE)[1:n]:
 * ties keep index order, missing values (NA_INTEGER for integers, NA or NaN
 * for doubles) are placed last in index order, and indices are returned as
 * double, since an integer vector cannot address the whole input. For parity
 * with the replaced radix path, 'hasna' plays no role here: missing values
 * are always placed last regardless of it.
 */

typedef struct {
  double   val;  /* element value; integer values are exact as doubles */
  R_xlen_t idx;  /* 0-based index into vec */
  bool     na;   /* TRUE when the element is missing */
} tnEntry;

/* TRUE if a comes before b in order(vec, decreasing = dcr, na.last = TRUE):
 * non-missing values precede missing ones (which are placed last), missing
 * values among themselves keep index order, and non-missing values are
 * ordered by 'decreasing' with ties keeping index order (stable, like
 * radix order(); -0.0 and 0.0 compare equal, and are both non-missing).
 */
static bool tnBefore(const tnEntry *restrict a, const tnEntry *restrict b, const bool dcr) {
  if (a->na != b->na) {
    return b->na;                      /* b missing and a not -> a first */
  }
  if (a->na) {
    return a->idx < b->idx;            /* missing entries keep index order */
  }
  if (a->val != b->val) {
    return dcr ? a->val > b->val : a->val < b->val;
  }
  return a->idx < b->idx;              /* ties keep index order */
}

/* Selection heap invariant: every child ranks before its parent, so the
 * root holds the entry that ranks LAST in the final order: the element
 * evicted when a better candidate arrives. 'hsize' is the current heap size.
 */
static void tnEvictSiftDown(tnEntry *restrict h, const R_xlen_t hsize, R_xlen_t i, const bool dcr) {
  for (;;) {
    const R_xlen_t l = 2 * i + 1;
    if (l >= hsize) {
      break;
    }
    const R_xlen_t r = l + 1;
    /* the child that ranks last: r ranks after l exactly when l ranks before r */
    R_xlen_t c = (r < hsize && tnBefore(h + l, h + r, dcr)) ? r : l;
    if (tnBefore(h + c, h + i, dcr)) {
      /* the last-ranked child ranks before the parent: parent already holds the worst spot */
      break;
    }
    const tnEntry t = h[i];
    h[i] = h[c];
    h[c] = t;
    i = c;
  }
}

/* Same elements are read through a data pointer when one is available; a
 * NULL pointer (unmaterialized ALTREP vector) falls back to element-wise
 * access, which compact integer and double sequences serve O(1) each
 * without materializing the vector.
 */
#define TN_INT_AT(p, v, i) ((p) ? (p)[i] : INTEGER_ELT(v, i))
#define TN_REAL_AT(p, v, i) ((p) ? (p)[i] : REAL_ELT(v, i))

static SEXP topnLong(SEXP vec, const R_xlen_t len0, const bool dcr) {
  const R_xlen_t len1 = xlength(vec);
  const SEXPTYPE tvec = UTYPEOF(vec);
  tnEntry *restrict h = (tnEntry*)R_alloc((size_t)len0, sizeof(tnEntry));
  const int *restrict pint = NULL;
  const double *restrict pdbl = NULL;
  if (tvec == INTSXP) {
    pint = INTEGER_OR_NULL(vec);
  } else {
    pdbl = REAL_OR_NULL(vec);
  }
  /* fill: the first len0 elements enter the heap one by one */
  R_xlen_t i;
  if (tvec == INTSXP) {
    for (i = 0; i < len0; ++i) {
      const int v = TN_INT_AT(pint, vec, i);
      h[i].val = (double)v;
      h[i].idx = i;
      h[i].na = v == NA_INTEGER;
    }
  } else {
    for (i = 0; i < len0; ++i) {
      const double v = TN_REAL_AT(pdbl, vec, i);
      h[i].val = v;
      h[i].idx = i;
      h[i].na = ISNAN(v);  /* covers NA_real_ and NaN, as in order() */
    }
  }
  for (R_xlen_t k = len0 / 2; k-- > 0; ) {
    tnEvictSiftDown(h, len0, k, dcr);
  }
  /* scan: a candidate replaces the worst kept element when it ranks before it;
   * missing candidates never do (their index is always larger) */
  if (tvec == INTSXP) {
    for (i = len0; i < len1; ++i) {
      const int v = TN_INT_AT(pint, vec, i);
      if (v == NA_INTEGER) {
        continue;
      }
      const tnEntry cand = {.val = (double)v, .idx = i, .na = false};
      if (tnBefore(&cand, h + 0, dcr)) {
        h[0] = cand;
        tnEvictSiftDown(h, len0, 0, dcr);
      }
    }
  } else {
    for (i = len0; i < len1; ++i) {
      const double v = TN_REAL_AT(pdbl, vec, i);
      if (ISNAN(v)) {
        continue;
      }
      const tnEntry cand = {.val = v, .idx = i, .na = false};
      if (tnBefore(&cand, h + 0, dcr)) {
        h[0] = cand;
        tnEvictSiftDown(h, len0, 0, dcr);
      }
    }
  }
  /* the heap already has the root-first 'worst' order, so a plain heapsort
   * with the same comparator orders all winners best-first in place */
  for (R_xlen_t e = len0 - 1; e > 0; --e) {
    const tnEntry t = h[0];
    h[0] = h[e];
    h[e] = t;
    tnEvictSiftDown(h, e, 0, dcr);
  }
  SEXP ans = PROTECT(allocVector(REALSXP, len0));
  double *restrict pans = REAL(ans);
  for (i = 0; i < len0; ++i) {
    pans[i] = (double)(h[i].idx + 1);
  }
  UNPROTECT(1);
  return ans;
}

static SEXP topnRadix(SEXP vec, const int len0, const bool dcr, SEXP env) {
  SEXP prem = PROTECT(callToOrder(vec, "radix", dcr, TRUE, env));
  SEXP ans = PROTECT(allocVector(INTSXP, len0));
  memcpy(INTEGER(ans), INTEGER(prem), len0 *sizeof(int));
  UNPROTECT(2);
  return ans;
}

/* The linear scan rescans all len0 candidates after every replacement, so
 * opposite-ordered input costs O(len1 * len0). Bail out to the radix path,
 * which already handles large n, once the rescan work exceeds a few full
 * passes over the input (#74). */
#define TN_RESCAN_OR_RADIX() \
  do { \
    work += len0; \
    if (work > budget) { \
      UNPROTECT(1); \
      return topnRadix(vec, len0, dcr, env); \
    } \
  } while (0)

SEXP topnR(SEXP vec, SEXP n, SEXP dec, SEXP hasna, SEXP env) {
  int len0 = asInteger(n);
  const R_xlen_t len1 = xlength(vec);
  if (isS4(vec)) {
    error("S4 class objects are not supported.");
  }
  if (len0 > len1) {
    warning("'n' is larger than length of 'vec'. 'n' will be set to length of 'vec'.");
    len0 = (int)len1;
  }
  if (len0 < 1) {
    error("Please enter a positive integer larger or equal to 1.");
  }
  if (!IS_BOOL(dec)) {
    error("Argument 'decreasing' must be TRUE or FALSE and length 1.");
  }
  if (!IS_BOOL(hasna)) {
    error("Argument 'hasna' must be TRUE or FALSE and length 1.");
  }
  const bool dcr = asLogical(dec);
  const SEXPTYPE tvec = UTYPEOF(vec);
  const bool vhasna = asLogical(hasna);
  // len1 > INT_MAX: ans is INTSXP and cannot hold indices into a long vector;
  // the bounded-heap selection below returns double indices like base::order()
  // with missing values last (order(na.last = TRUE) semantics) and uses memory
  // proportional to n instead of a full-length order() index (~8*len1 bytes,
  // ~17 GB for 2^31 elements) (#83). Unlike order(), it also supports
  // decreasing = TRUE, which errors on long vectors in base R.
  if (len1 > INT_MAX && (tvec == INTSXP || tvec == REALSXP)) {
    return topnLong(vec, len0, dcr);
  }
  if ( ((len0 > 2000 && !vhasna) || (len0 > 1500 && vhasna)) && (tvec == INTSXP || tvec == REALSXP)) {
    return topnRadix(vec, len0, dcr, env);
  }
  const R_xlen_t budget = 8 * len1;
  R_xlen_t work = 0;
  SEXP ans = PROTECT(allocVector(INTSXP, len0));
  int *restrict pans = INTEGER(ans);
  int tmp;
  if (dcr) {
    switch(tvec) {
    case INTSXP: {
      int i, j, idx = 0;
      const int *restrict pvec = INTEGER(vec);
      int min_value = pvec[0];
      if (vhasna) {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] <= min_value || pvec[i] == NA_INTEGER) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] == NA_INTEGER) {
            continue;
          }
          if (pvec[i] > min_value) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if ((min_value > pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) || pvec[pans[j]] == NA_INTEGER) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] > pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1])); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      } else {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] <= min_value) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] > min_value) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if (min_value > pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] > pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1])); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      }
    } break;
    case REALSXP: {
      int i, j, idx = 0;
      const double *restrict pvec = REAL(vec);
      double min_value = pvec[0];
      if (vhasna) {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] <= min_value || ISNAN(pvec[i])) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (ISNAN(pvec[i])) {
            continue;
          }
          if (pvec[i] > min_value || ISNAN(min_value)) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if ((min_value > pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) || ISNAN(pvec[pans[j]])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] > pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1]) || (!ISNAN(pvec[tmp]) && ISNAN(pvec[pans[j-1]]))); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      } else {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] <= min_value) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] > min_value) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if (min_value > pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] > pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1])); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      }
    } break;
    default:
      error("Type %s is not supported.", type2char(tvec));
    }
  } else {
    switch(tvec) {
    case INTSXP: {
      int i, j, idx = 0;
      const int *restrict pvec = INTEGER(vec);
      int min_value = pvec[0];
      if (vhasna) {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if ((pvec[i] >= min_value && min_value != NA_INTEGER) || pvec[i] == NA_INTEGER) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] == NA_INTEGER) {
            continue;
          }
          if (pvec[i] < min_value || min_value == NA_INTEGER) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if (((min_value < pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) && min_value != NA_INTEGER) || pvec[pans[j]] == NA_INTEGER) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          if (pvec[tmp] == NA_INTEGER) {
            continue;
          }
          for (j = i; j > 0 && (pvec[tmp] < pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1]) || pvec[pans[j-1]] == NA_INTEGER); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      } else {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] >= min_value) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] < min_value) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if (min_value < pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] < pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1])); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      }
    } break;
    case REALSXP: {
      int i, j, idx = 0;
      const double *restrict pvec = REAL(vec);
      double min_value = pvec[0];
      if (vhasna) {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] >= min_value || ISNAN(pvec[i])) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (ISNAN(pvec[i])) {
            continue;
          }
          if (pvec[i] < min_value || ISNAN(min_value)) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if ((min_value < pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) || ISNAN(pvec[pans[j]])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] < pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1]) || (!ISNAN(pvec[tmp]) && ISNAN(pvec[pans[j-1]]))); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      } else {
        for (i = 0; i < len0; ++i) {
          pans[i] = i;
          if (pvec[i] >= min_value) {
            min_value = pvec[i];
            idx = i;
          }
        }
        for (i = len0; i < len1; ++i) {
          if (pvec[i] < min_value) {
            min_value = pvec[i];
            pans[idx] = i;
            TN_RESCAN_OR_RADIX();
            for (j = 0; j <len0; ++j) {
              if (min_value < pvec[pans[j]] || (min_value == pvec[pans[j]] && pans[idx] < pans[j])) {
                min_value = pvec[pans[j]];
                idx = j;
              }
            }
          }
        }
        for (i = 0; i < len0; ++i) {
          tmp = pans[i];
          for (j = i; j > 0 && (pvec[tmp] < pvec[pans[j-1]] || (pvec[tmp] == pvec[pans[j-1]] && tmp < pans[j-1])); --j) {
            pans[j] = pans[j-1];
          }
          pans[j] = tmp;
        }
        for (i =0; i < len0; ++i) {
          pans[i]++;
        }
      }
    } break;
    default:
      error("Type %s is not supported.", type2char(tvec));
    }
  }
  UNPROTECT(1);
  return ans;
}
