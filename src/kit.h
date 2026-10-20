#ifdef _OPENMP
  #include<omp.h>
  #define omp_enabled true
  #define max_thread omp_get_num_procs()
  #define min_thread 1
  #define OMP_PARALLEL_FOR(nth) _Pragma("omp parallel for num_threads(nth)")
#else
  #define omp_enabled false
  #define max_thread 1
  #define min_thread 1
  #define omp_get_thread_num() 0
  #define OMP_PARALLEL_FOR(n)
#endif

#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rversion.h>

#if !defined(R_VERSION) || R_VERSION < R_Version(3, 5, 0)
  #define USE_RINTERNALS
  #define DATAPTR_RO(x) ((const void *)DATAPTR(x))
#endif

#include <Rinternals.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#if R_VERSION < R_Version(4, 5, 0)
# define isDataFrame(x) Rf_isFrame(x)
#endif

#if !defined SSIZE_MAX
  #define SSIZE_MAX LLONG_MAX
#endif

#ifdef WIN32
  #include <windows.h>
#else
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

#define UTYPEOF(x) ((unsigned)TYPEOF(x))
#define IS_BOOL(x) (XLENGTH(x)==1 && TYPEOF(x)==LGLSXP && LOGICAL(x)[0]!=NA_LOGICAL)
#define IS_VALID_TYPE(x) ((x) == LGLSXP || (x)==INTSXP || (x)==REALSXP || (x)==CPLXSXP || (x)==STRSXP || (x)==VECSXP)
#define PTR_ETL(x, y) (((const SEXP *)DATAPTR_RO(x))[y])
#define SEXPPTR_RO(x) ((const SEXP *)DATAPTR_RO(x))
#define ISNA_COMPLEX(x) (ISNA(x.r) || ISNA(x.i))
#define ISNAN_COMPLEX(x) (ISNAN(x.r) || ISNAN(x.i))
#define EQUAL_CPLX(x, y) (((x.r) == (y.r)) && ((x.i) == (y.i)))
#define RCHAR(x, y) CHAR(STRING_ELT(x, y))
#define SEXP_F ScalarLogical(FALSE)
#define SEXP_T ScalarLogical(TRUE)
#define HASH(key, K)  (3141592653U * (unsigned int)(key) >> (32 - (K)))
#define N_ISNAN(x, y) (!ISNAN(x) && !ISNAN(y))
#define B_IsNA(x, y)  (R_IsNA(x) && R_IsNA(y))
#define B_IsNaN(x, y) (R_IsNaN(x) && R_IsNaN(y))
#define B_ISNAN(x, y) (ISNAN(x) && ISNAN(y))
#define C_IsNA(x)     (R_IsNA(x.r) || R_IsNA(x.i))
#define C_IsNaN(x)    (R_IsNaN(x.r) || R_IsNaN(x.i))
#define C_ISNAN(x, y) (B_ISNAN(x, y) || (N_ISNAN(x, y) && x == y))
#define REQUAL(x, y)  (N_ISNAN(x, y) ? (x == y) : (B_IsNA(x, y) || B_IsNaN(x, y)))
#define CEQUAL(x, y) ((N_ISNAN(x.r, x.i) && N_ISNAN(y.r, y.i)) ? (x.r == y.r && x.i == y.i) : (C_IsNA(x) ? C_IsNA(y) : (C_IsNA(y) ? 0 : (C_ISNAN(x.r, y.r) && C_ISNAN(x.i, y.i)))))

union uno { double d; unsigned int u[2]; };

/*
 * Growable duplicate tables. Each table stores 1-based input indices in an
 * open-addressed int array and grows at 75% load. Rehashing re-reads the
 * source vector, so the table itself stays 4 bytes per slot.
 */

typedef struct {
  const void *px;
  R_xlen_t len_i;
  R_xlen_t len_x;
} kit_row_ctx;

typedef int *(*kit_row_grow_fn)(const kit_row_ctx *ctx, int *h, size_t M, size_t M2, int K2);

static inline size_t kit_hash_rows_lgl(const int *px, const R_xlen_t i,
                                       const R_xlen_t len_i, const R_xlen_t len_x,
                                       const int K) {
  size_t id = 0;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    id ^= ((unsigned)(j+1) * ((px[i+j*len_i] == NA_LOGICAL) ? 2U : (size_t) px[i+j*len_i]))*97*(j+1);
  }
  return HASH(id, K);
}

static inline size_t kit_hash_rows_int(const int *px, const R_xlen_t i,
                                       const R_xlen_t len_i, const R_xlen_t len_x,
                                       const int K) {
  R_xlen_t key = 0;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    key ^= HASH(((px[i+j*len_i] == NA_INTEGER) ? 0 : px[i+j*len_i]), K)*97*(j+1);
  }
  return HASH(key, K);
}

static inline size_t kit_hash_rows_int_raw(const int *px, const R_xlen_t i,
                                           const R_xlen_t len_i, const R_xlen_t len_x,
                                           const int K) {
  R_xlen_t key = 0;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    key ^= HASH(px[i+j*len_i], K)*97*(j+1);
  }
  return HASH(key, K);
}

static inline size_t kit_hash_rows_real(const double *px, const R_xlen_t i,
                                        const R_xlen_t len_i, const R_xlen_t len_x,
                                        const int K) {
  R_xlen_t key = 0;
  union uno tpv;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    tpv.d = px[i+j*len_i];
    key ^= HASH(tpv.u[0] + tpv.u[1], K)*97*(j+1);
  }
  tpv.d = key;
  return HASH(tpv.u[0] + tpv.u[1], K);
}

static inline size_t kit_hash_rows_cplx(const Rcomplex *px, const R_xlen_t i,
                                        const R_xlen_t len_i, const R_xlen_t len_x,
                                        const int K) {
  R_xlen_t key = 0;
  unsigned int u;
  union uno tpv;
  Rcomplex tmp;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    tmp.r = (px[i+j*len_i].r == 0.0) ? 0.0 : px[i+j*len_i].r;
    tmp.i = (px[i+j*len_i].i == 0.0) ? 0.0 : px[i+j*len_i].i;
    if (C_IsNA(tmp)) {
      tmp.r = tmp.i = NA_REAL;
    } else if (C_IsNaN(tmp)) {
      tmp.r = tmp.i = R_NaN;
    }
    tpv.d = tmp.r;
    u = tpv.u[0] ^ tpv.u[1];
    tpv.d = tmp.i;
    u ^= tpv.u[0] ^ tpv.u[1];
    key ^= HASH(u, K)*97*(j+1);
  }
  return HASH(key, K);
}

static inline size_t kit_hash_rows_str(const SEXP *px, const R_xlen_t i,
                                       const R_xlen_t len_i, const R_xlen_t len_x,
                                       const int K) {
  R_xlen_t key = 0;
  for (R_xlen_t j = 0; j < len_x; ++j) {
    key ^= HASH(((intptr_t) px[i+j*len_i] & 0xffffffff), K)*97*(j+1);
  }
  return HASH(key, K);
}

#define KIT_DEFINE_ROW_GROW(name, type, hashfn) \
static inline int *kit_grow_rows_##name(const kit_row_ctx *ctx, int *h, \
                                        const size_t M, const size_t M2, \
                                        const int K2) { \
  const type *px = (const type*) ctx->px; \
  int *h2 = (int*) R_Calloc(M2, int); \
  for (size_t s = 0; s < M; ++s) { \
    const int e = h[s]; \
    if (e) { \
      const int i = e - 1; \
      size_t id = hashfn(px, i, ctx->len_i, ctx->len_x, K2); \
      while (h2[id]) { id++; id %= M2; } \
      h2[id] = e; \
    } \
  } \
  R_Free(h); \
  return h2; \
}

KIT_DEFINE_ROW_GROW(lgl, int, kit_hash_rows_lgl)
KIT_DEFINE_ROW_GROW(int, int, kit_hash_rows_int)
KIT_DEFINE_ROW_GROW(int_raw, int, kit_hash_rows_int_raw)
KIT_DEFINE_ROW_GROW(real, double, kit_hash_rows_real)
KIT_DEFINE_ROW_GROW(cplx, Rcomplex, kit_hash_rows_cplx)
KIT_DEFINE_ROW_GROW(str, SEXP, kit_hash_rows_str)

#define KIT_GROW_ROWS() do { if (count > M - (M >> 2)) { \
  size_t Mnew = M << 2; int Knew = K + 2; \
  if (Mnew > Mbase) { Mnew = Mbase; Knew = Kbase; } \
  if (Mnew > M) { h = grow(&rowctx, h, M, Mnew, Knew); M = Mnew; K = Knew; } \
} } while (0)
static inline int *kit_rehash_int(const int *restrict px, int *restrict h,
                                  const size_t M, const size_t M2,
                                  const int K2) {
  int *restrict h2 = (int*) R_Calloc(M2, int);
  for (size_t s = 0; s < M; ++s) {
    const int e = h[s];
    if (e) {
      const int i = e - 1;
      size_t id = (px[i] == NA_INTEGER) ? 0 : HASH(px[i], K2);
      while (h2[id]) { id++; id %= M2; }
      h2[id] = e;
    }
  }
  R_Free(h);
  return h2;
}

static inline int *kit_rehash_real(const double *restrict px, int *restrict h,
                                   const size_t M, const size_t M2,
                                   const int K2) {
  int *restrict h2 = (int*) R_Calloc(M2, int);
  union uno tpv;
  for (size_t s = 0; s < M; ++s) {
    const int e = h[s];
    if (e) {
      const int i = e - 1;
      tpv.d = R_IsNA(px[i]) ? NA_REAL : (R_IsNaN(px[i]) ? R_NaN : px[i]);
      size_t id = HASH(tpv.u[0] + tpv.u[1], K2);
      while (h2[id]) { id++; id %= M2; }
      h2[id] = e;
    }
  }
  R_Free(h);
  return h2;
}

static inline int *kit_rehash_cplx(const Rcomplex *restrict px, int *restrict h,
                                   const size_t M, const size_t M2,
                                   const int K2) {
  int *restrict h2 = (int*) R_Calloc(M2, int);
  union uno tpv;
  Rcomplex tmp;
  for (size_t s = 0; s < M; ++s) {
    const int e = h[s];
    if (e) {
      const int i = e - 1;
      tmp.r = (px[i].r == 0.0) ? 0.0 : px[i].r;
      tmp.i = (px[i].i == 0.0) ? 0.0 : px[i].i;
      if (C_IsNA(tmp)) {
        tmp.r = tmp.i = NA_REAL;
      } else if (C_IsNaN(tmp)) {
        tmp.r = tmp.i = R_NaN;
      }
      tpv.d = tmp.r;
      unsigned int u = tpv.u[0] ^ tpv.u[1];
      tpv.d = tmp.i;
      u ^= tpv.u[0] ^ tpv.u[1];
      size_t id = HASH(u, K2);
      while (h2[id]) { id++; id %= M2; }
      h2[id] = e;
    }
  }
  R_Free(h);
  return h2;
}

static inline int *kit_rehash_str(const SEXP *restrict px, int *restrict h,
                                  const size_t M, const size_t M2,
                                  const int K2) {
  int *restrict h2 = (int*) R_Calloc(M2, int);
  for (size_t s = 0; s < M; ++s) {
    const int e = h[s];
    if (e) {
      const int i = e - 1;
      size_t id = HASH(((intptr_t) px[i] & 0xffffffff), K2);
      while (h2[id]) { id++; id %= M2; }
      h2[id] = e;
    }
  }
  R_Free(h);
  return h2;
}

#define KIT_GROW_INT()  do { if (count > M - (M >> 2)) { \
  size_t Mnew = M << 2; int Knew = K + 2; \
  if (Mnew > Mbase) { Mnew = Mbase; Knew = Kbase; } \
  if (Mnew > M) { h = kit_rehash_int(px, h, M, Mnew, Knew); M = Mnew; K = Knew; } \
} } while (0)
#define KIT_GROW_REAL() do { if (count > M - (M >> 2)) { \
  size_t Mnew = M << 2; int Knew = K + 2; \
  if (Mnew > Mbase) { Mnew = Mbase; Knew = Kbase; } \
  if (Mnew > M) { h = kit_rehash_real(px, h, M, Mnew, Knew); M = Mnew; K = Knew; } \
} } while (0)
#define KIT_GROW_CPLX() do { if (count > M - (M >> 2)) { \
  size_t Mnew = M << 2; int Knew = K + 2; \
  if (Mnew > Mbase) { Mnew = Mbase; Knew = Kbase; } \
  if (Mnew > M) { h = kit_rehash_cplx(px, h, M, Mnew, Knew); M = Mnew; K = Knew; } \
} } while (0)
#define KIT_GROW_STR()  do { if (count > M - (M >> 2)) { \
  size_t Mnew = M << 2; int Knew = K + 2; \
  if (Mnew > Mbase) { Mnew = Mbase; Knew = Kbase; } \
  if (Mnew > M) { h = kit_rehash_str(px, h, M, Mnew, Knew); M = Mnew; K = Knew; } \
} } while (0)
#define STR_DF mkString("data.frame")
#define MAX(a,b) (((a)>(b))?(a):(b))
#define IS_LOGICAL(x) (isLogical(x) && XLENGTH(x)==1)

extern SEXP addColToDataFrame(SEXP df, SEXP mcol, SEXP coln);
extern SEXP callToOrder (SEXP x, const char* method, bool desc, bool na, SEXP env);
extern SEXP charToFactR(SEXP x, SEXP decreasingArg, SEXP nthread, SEXP nalast, SEXP env, SEXP addNA);
extern SEXP countR(SEXP x, SEXP y);
extern SEXP countNAR(SEXP x);
extern SEXP countOccurR(SEXP x);
extern SEXP countOccurDataFrameR(SEXP x);
extern SEXP cpsortR(SEXP x, SEXP decreasing, SEXP nthread, SEXP nalast, SEXP env, SEXP index, SEXP clocale);
extern SEXP dfToMatrix(SEXP df);
extern SEXP dupR(SEXP x, SEXP uniq, SEXP fromLast);
extern SEXP dupVecR(SEXP x, SEXP uniq, SEXP fromLast);
extern SEXP dupVecIndexOnlyR(SEXP x);
extern SEXP dupDataFrameR(SEXP x, SEXP uniq, SEXP fromLast);
extern SEXP dupMatrixR(SEXP x, SEXP uniq, bool idx, SEXP fromLast);
extern SEXP dupLenR(SEXP x);
extern SEXP dupLenDataFrameR(SEXP x);
extern SEXP dupLenMatrixR(SEXP x);
extern SEXP dupLenVecR(SEXP x);
extern SEXP fposR(SEXP needle, SEXP haystack, SEXP all, SEXP overlap);
extern SEXP fposMatR(SEXP needle, SEXP haystack, SEXP all, SEXP overlap);
extern SEXP fposVectR(SEXP ndle, SEXP hsk, SEXP all, SEXP overlap);
extern SEXP iifR(SEXP l, SEXP a, SEXP b, SEXP na, SEXP tprom, SEXP nthreads);
extern SEXP nifR(SEXP na, SEXP rho, SEXP args);
extern SEXP nifInternalR(SEXP na, SEXP args);
extern SEXP nswitchR(SEXP x, SEXP na, SEXP nthreads, SEXP chkenc, SEXP args);
extern SEXP ompEnabledR(void);
extern SEXP pallR(SEXP na, SEXP args);
extern SEXP panyR(SEXP na, SEXP args);
extern SEXP pcountR(SEXP x, SEXP args);
extern SEXP pcountNAR(SEXP args);
extern SEXP pfirstR(SEXP last, SEXP args);
extern SEXP fpminR(SEXP na, SEXP args);
extern SEXP fpmaxR(SEXP na, SEXP args);
extern SEXP pmeanR(SEXP na, SEXP args);
extern SEXP pprodR(SEXP na, SEXP args);
extern SEXP prangeR(SEXP na, SEXP args);
extern SEXP psumR(SEXP na, SEXP args);
extern SEXP setlevelsR(SEXP x, SEXP old_lvl, SEXP new_lvl, SEXP skip_absent);
extern SEXP subSetColDataFrame(SEXP df, SEXP str);
extern SEXP subSetColMatrix(SEXP x, R_xlen_t idx);
extern SEXP subSetRowDataFrame(SEXP df, SEXP rws);
extern SEXP subSetRowMatrix(SEXP mat, SEXP rws);
extern SEXP topnR(SEXP vec, SEXP n, SEXP dec, SEXP hasna, SEXP env);
extern SEXP vswitchR(SEXP x, SEXP values, SEXP outputs, SEXP na, SEXP nthreads, SEXP chkenc);

extern SEXP createMappingObjectR(SEXP MapName, SEXP MapLength, SEXP DataObject, SEXP verboseArg);
extern SEXP getMappingObjectR(SEXP MapName, SEXP MapLength, SEXP verboseArg);
extern SEXP clearMappingObjectR(SEXP ext, SEXP verboseArg);

bool isMixEnc(SEXP x);
SEXP enc2UTF8(SEXP x);
