# Long-vector and ALTREP boundary tests for the XLENGTH/R_xlen_t audit (issue #61).
#
# The checks at the top are cheap: lengths are validated before any input is
# materialized or read, so nothing large is ever allocated.
#
# The >INT_MAX round trips at the bottom are gated behind
# KIT_TEST_LONG_VECTOR=true (e.g. KIT_TEST_LONG_VECTOR=true R CMD check on a
# big-memory machine). The topn() selection itself now needs memory proportional
# to n instead of a full-length order() index (issue #83): an ALTREP input can
# even be read without materializing, while the materialized read-path cases
# below build a 17 GB double or 9 GB integer vector and need the RAM for it.

# --- cheap: long lengths are reported / refused without allocating ---

# psum compares lengths with xlength() before reading data: the >INT_MAX length
# must appear in full, not truncated to int.
expect_error(
  psum(1:3e9, 1:10, na.rm = FALSE),
  pattern = "Argument 2 is of length 10 but argument 1 is of length 3000000000",
  fixed = TRUE,
  info = "longv-0001.001"
)

# funique/fduplicated/uniqLen refuse long vectors before materializing the ALTREP sequence
expect_error(funique(1:3e9), pattern = "Long vector not supported yet", fixed = TRUE, info = "longv-0001.002")
expect_error(fduplicated(1:3e9), pattern = "Long vector not supported yet", fixed = TRUE, info = "longv-0001.003")
expect_error(uniqLen(1:3e9), pattern = "Long vector not supported yet", fixed = TRUE, info = "longv-0001.004")

# --- ALTREP (compact integer sequence) inputs on normal-size paths ---

expect_identical(psum(seq_len(10000L), seq_len(10000L), na.rm = TRUE), 2L * seq_len(10000L), info = "longv-0001.005")
expect_identical(funique(seq_len(10000L)), seq_len(10000L), info = "longv-0001.006")
expect_identical(uniqLen(seq_len(10000L)), 10000L, info = "longv-0001.007")
expect_identical(topn(seq_len(10000L), 5L, decreasing = FALSE, hasna = FALSE), 1:5, info = "longv-0001.008")

# classed long vectors must not silently take the integer-index order() path:
# the xtfrm result is checked before selection and a non-numeric/short result
# errors. seq_len() is a compact ALTREP sequence, so this allocates nothing.
nlong <- 2^31 + 1000000
xl <- structure(seq_len(nlong), class = "kit_topn_long_bad")
registerS3method("xtfrm", "kit_topn_long_bad", function(x) 1L)
expect_error(topn(xl, 2L), pattern = "Classed long vectors are not supported by 'topn()'.",
             fixed = TRUE, info = "longv-0001.126")
rm(xl, nlong)

# a data.frame with > INT_MAX cells is refused up front. All columns share one
# small logical vector, so building it is cheap and the error fires before
# dfToMatrix allocates the matrix.
col <- rep(TRUE, 50000L)
bigdf <- structure(rep(list(col), 50000L), names = paste0("V", seq_len(50000L)),
                   class = "data.frame", row.names = c(NA_integer_, -50000L))
expect_error(funique(bigdf), pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.009")
expect_error(uniqLen(bigdf), pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.010")

# the mixed-type data.frame branch has no dfToMatrix guard: countOccur has no
# same-type shortcut at all, so a >INT_MAX-cell mixed-regime data.frame used to
# run the whole mixed-type pipeline (an INTSXP matrix of 2.5e9 cells). fduplicated
# shares funique's same-type branch and errors through dfToMatrix like above.
expect_error(fduplicated(bigdf), pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.011")
expect_error(countOccur(bigdf),  pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.012")
rm(col, bigdf)

# more than INT_MAX rows on a data.frame (issue #82): row counts are truncated to
# int inside allocMatrix(INTSXP, (int) rows, (int) cols). Column 1 is a compact
# sequence ALTREP, so building the data.frame allocates almost nothing: the error
# must fire before any column is read, and no column is ever dereferenced below,
# so column 2 (an integer(0), shorter than column 1) is first kept small on
# purpose. R itself cannot attach integer row.names above INT_MAX, so such
# data.frames exist only via attribute surgery. Master errored with R's native
# "negative extents to matrix" (for (int) rows < 0) instead of the standard error.
rows <- 2^31 + 1000000L
mixeddf <- structure(list(seq_len(rows), integer(0)), names = c("a", "b"), class = "data.frame")
expect_error(funique(mixeddf),     pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.013")
expect_error(fduplicated(mixeddf), pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.014")
expect_error(uniqLen(mixeddf),     pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.015")
expect_error(countOccur(mixeddf),  pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.016")
rm(mixeddf, rows)

# --- opt-in: >INT_MAX round trips ---
if (!identical(Sys.getenv("KIT_TEST_LONG_VECTOR"), "true")) {
  exit_file("KIT_TEST_LONG_VECTOR != true: skipping >INT_MAX round-trip tests (most functions here allocate 10-17 GB; topn's ALTREP checks are cheap but gated along)")
}

n <- 2^31 + 1000000 # double, as length() returns for long vectors

# psum: integer path on a long vector (x is shared between the two arguments)
x <- rep(1L, n)
r <- psum(x, x, na.rm = TRUE)
expect_identical(length(r), n, info = "longv-0001.101")
expect_identical(r[c(1, n)], c(2L, 2L), info = "longv-0001.102")
rm(x, r)
invisible(gc())

# funique/uniqLen: long factor keeps distinct factor values in both directions
f <- structure(rep(1L, n), levels = c("a", "b"), class = "factor")
expect_identical(funique(f), f[1L], info = "longv-0001.103")
expect_identical(funique(f, fromLast = TRUE), f[1L], info = "longv-0001.104")
expect_identical(uniqLen(f), 1L, info = "longv-0001.105")
rm(f)
invisible(gc())

# funique/uniqLen: long logical
lg <- rep(TRUE, n)
expect_identical(funique(lg), TRUE, info = "longv-0001.106")
expect_identical(uniqLen(lg), 1L, info = "longv-0001.107")
expect_error(fduplicated(lg), pattern = "Long vector not supported yet",
             fixed = TRUE, info = "longv-0001.125")
rm(lg)
invisible(gc())

# mixed-type data.frame with more than INT_MAX rows, on a real construction with
# matched column lengths (issue #82): the guard must fire before any column is
# read, so the 17 GB integer column is built and then never even copied. The
# first column is a compact sequence ALTREP; R cannot attach integer row.names
# above INT_MAX, so attribute surgery is the only way to build this.
mixeddf <- structure(list(seq_len(n), rep(1L, n)), names = c("a", "b"), class = "data.frame")
expect_error(funique(mixeddf),     pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.121")
expect_error(fduplicated(mixeddf), pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.122")
expect_error(uniqLen(mixeddf),     pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.123")
expect_error(countOccur(mixeddf),  pattern = "Length of 'x' is too large. (Long vector not supported yet)", fixed = TRUE, info = "longv-0001.124")
rm(mixeddf)
invisible(gc())

# psort: long character vectors are refused before being read
s <- rep("a", n)
expect_error(psort(s), pattern = "Long vector not supported yet", fixed = TRUE, info = "longv-0001.108")
rm(s)
invisible(gc())

# topn: long input takes the bounded-heap selection and returns double indices
# like base::order() (#83). The sequenced input below is an ALTREP compact
# double object that keeps unmaterialized until read; the scan handles it with
# element-wise access, so memory stays proportional to n throughout.
v <- seq_len(n)
r <- topn(v, 5L, decreasing = FALSE, hasna = FALSE)
expect_identical(typeof(r), "double", info = "longv-0001.109")
expect_identical(as.integer(r), 1:5, info = "longv-0001.110")

# decreasing direction: the n largest elements are the tail of the sequence,
# in stable index order (all distinct values), best first like order()
r <- topn(v, 5L, decreasing = TRUE, hasna = FALSE)
expect_identical(as.numeric(r), as.numeric(n:(n - 4)), info = "longv-0001.111")
# hasna is accepted for API parity but plays no role when there are no NAs
expect_identical(topn(v, 5L, decreasing = TRUE, hasna = TRUE), r, info = "longv-0001.112")
# n = 1 is the long-vector equivalent of which.max()/which.min()
expect_identical(topn(v, 1L, decreasing = TRUE, hasna = TRUE)[[1L]], n, info = "longv-0001.113")
expect_identical(topn(v, 1L, decreasing = FALSE, hasna = TRUE)[[1L]], 1.0, info = "longv-0001.114")
rm(v, r)
invisible(gc())

# topn on a materialized long vector (data-pointer read path) with ties and
# missing values. The pattern repeats every 5 elements: positions 1 mod 5
# hold NA, 2 mod 5 hold NaN, 3 mod 5 and 0 mod 5 hold 1.5, and 4 mod 5 holds
# -2.5. Values are plentiful, so missing values never reach the output: the
# winners are the earliest ties of the extreme value in each direction
# (the two 1.5-carrying position classes interleave as 3, 5, 8, 10, 13, 15
# when decreasing; the -2.5 class runs 4, 9, 14, 19, 24, 29 when increasing).
p <- rep(c(NA_real_, NaN, 1.5, -2.5, 1.5), length.out = n)
expect_identical(topn(p, 6L, decreasing = TRUE, hasna = TRUE),
  as.numeric(c(3, 5, 8, 10, 13, 15)), info = "longv-0001.115")
expect_identical(topn(p, 6L, decreasing = FALSE, hasna = FALSE),
  as.numeric(c(4, 9, 14, 19, 24, 29)), info = "longv-0001.116")
rm(p)
invisible(gc())

# topn on a materialized long vector with fewer non-missing values than n:
# output is all m values (ties in index order) followed by the earliest NAs
q <- rep(NA_real_, n)
q[seq(1, n, by = 1e9)] <- 1.0
expect_identical(topn(q, 5L, decreasing = TRUE, hasna = TRUE),
  as.numeric(c(seq(1, n, by = 1e9), 2, 3)), info = "longv-0001.117")
rm(q)
invisible(gc())

# materialized long integer vector with ties (NA_INTEGER is missing, 3 ties
# repeat every 3 elements: NA, 3, -1)
w <- rep(c(NA_integer_, 3L, -1L), length.out = n)
expect_identical(topn(w, 2L, decreasing = TRUE, hasna = TRUE),
  as.numeric(c(2, 5)), info = "longv-0001.118")
expect_identical(topn(w, 3L, decreasing = FALSE, hasna = FALSE),
  as.numeric(c(3, 6, 9)), info = "longv-0001.119")
rm(w)
invisible(gc())

# long integer vector with fewer non-missing values than n: values first, then
# the earliest NA_INTEGER elements
w <- rep(NA_integer_, n)
w[seq(1, n, by = 1e9)] <- 7L
expect_identical(topn(w, 5L, decreasing = TRUE, hasna = FALSE),
  as.numeric(c(seq(1, n, by = 1e9), 2, 3)), info = "longv-0001.120")
rm(w)
invisible(gc())
