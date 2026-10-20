# Tests for topn() — indices of top-n elements — tinytest (see issue #54).
# Migrated from testthat 3e to tinytest to keep kit lean (zero test dependencies).
# Each legacy check("id", actual, expected) maps to expect_identical(..., info="id");
# 31 tolerant cases (identical FALSE but all.equal+typeof TRUE) use expect_kit_equal(..., info="id");
# check(..., error=) maps to expect_error(pattern=, fixed=TRUE, info="id").
# Legacy IDs are preserved as info= labels (prefixed by function for uniqueness).
# Setup and expectations stay interleaved in legacy order: later sections may
# redefine x/y/out_vec/... so setup must not be hoisted above earlier tests.

sys.source("helper-kit.R", envir = environment())
set.seed(123)

x0 = c(3L, 2L, 10L, NA_integer_, 1L, 1L, NA_integer_,  NA_integer_, 10L, 20L, 20L, 20L, 30L)
x1 = as.numeric(x0)
x2 = c(NA_integer_, NA_integer_, NA_integer_)
x3 = as.numeric(x2)
x4 = as.raw(c(1,2,3))
x5 = sample(c(1:1000),3e3,TRUE)
x6 = sample(as.numeric(c(1:1000)),1e3,TRUE)
class2134 = setClass("class2134", slots=list(x="numeric"))
s1 = class2134(x=20191231)
x7 = seq.int(1e4)

  expect_identical(topn(x0, 1L, decreasing=FALSE), order(x0)[1:1], info="topn-0001.001")


  expect_identical(topn(x0, 2L, decreasing=FALSE), order(x0)[1:2], info="topn-0001.002")


  expect_identical(topn(x0, 3L, decreasing=FALSE), order(x0)[1:3], info="topn-0001.003")


  expect_identical(topn(x0, 4L, decreasing=FALSE), order(x0)[1:4], info="topn-0001.004")


  expect_identical(topn(x0, 5L, decreasing=FALSE), order(x0)[1:5], info="topn-0001.005")


  expect_identical(topn(x0, 6L, decreasing=FALSE), order(x0)[1:6], info="topn-0001.006")


  expect_identical(topn(x0, 7L, decreasing=FALSE), order(x0)[1:7], info="topn-0001.007")


  expect_identical(topn(x0, 8L, decreasing=FALSE), order(x0)[1:8], info="topn-0001.008")


  expect_identical(topn(x0, 9L, decreasing=FALSE), order(x0)[1:9], info="topn-0001.009")


  expect_identical(topn(x0, 10L, decreasing=FALSE), order(x0)[1:10], info="topn-0001.010")


  expect_identical(topn(x0, 11L, decreasing=FALSE), order(x0)[1:11], info="topn-0001.011")


  expect_identical(topn(x0, 12L, decreasing=FALSE), order(x0)[1:12], info="topn-0001.012")


  expect_identical(topn(x0, 13L, decreasing=FALSE), order(x0)[1:13], info="topn-0001.013")


  expect_identical(topn(x1, 1L, decreasing=FALSE), order(x1)[1:1], info="topn-0001.014")


  expect_identical(topn(x1, 2L, decreasing=FALSE), order(x1)[1:2], info="topn-0001.015")


  expect_identical(topn(x1, 3L, decreasing=FALSE), order(x1)[1:3], info="topn-0001.016")


  expect_identical(topn(x1, 4L, decreasing=FALSE), order(x1)[1:4], info="topn-0001.017")


  expect_identical(topn(x1, 5L, decreasing=FALSE), order(x1)[1:5], info="topn-0001.018")


  expect_identical(topn(x1, 6L, decreasing=FALSE), order(x1)[1:6], info="topn-0001.019")


  expect_identical(topn(x1, 7L, decreasing=FALSE), order(x1)[1:7], info="topn-0001.020")


  expect_identical(topn(x1, 8L, decreasing=FALSE), order(x1)[1:8], info="topn-0001.021")


  expect_identical(topn(x1, 9L, decreasing=FALSE), order(x1)[1:9], info="topn-0001.022")


  expect_identical(topn(x1, 10L, decreasing=FALSE), order(x1)[1:10], info="topn-0001.023")


  expect_identical(topn(x1, 11L, decreasing=FALSE), order(x1)[1:11], info="topn-0001.024")


  expect_identical(topn(x1, 12L, decreasing=FALSE), order(x1)[1:12], info="topn-0001.025")


  expect_identical(topn(x1, 13L, decreasing=FALSE), order(x1)[1:13], info="topn-0001.026")


  expect_identical(topn(x2, 1L, decreasing=FALSE), order(x2)[1:1], info="topn-0001.027")


  expect_identical(topn(x2, 2L, decreasing=FALSE), order(x2)[1:2], info="topn-0001.028")


  expect_identical(topn(x2, 3L, decreasing=FALSE), order(x2)[1:3], info="topn-0001.029")


  expect_identical(topn(x3, 1L, decreasing=FALSE), order(x3)[1:1], info="topn-0001.030")


  expect_identical(topn(x3, 2L, decreasing=FALSE), order(x3)[1:2], info="topn-0001.031")


  expect_identical(topn(x3, 3L, decreasing=FALSE), order(x3)[1:3], info="topn-0001.032")


  expect_identical(topn(x0, 1L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:1], info="topn-0001.033")


  expect_identical(topn(x0, 2L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:2], info="topn-0001.034")


  expect_identical(topn(x0, 3L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:3], info="topn-0001.035")


  expect_identical(topn(x0, 4L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:4], info="topn-0001.036")


  expect_identical(topn(x0, 5L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:5], info="topn-0001.037")


  expect_identical(topn(x0, 6L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:6], info="topn-0001.038")


  expect_identical(topn(x0, 7L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:7], info="topn-0001.039")


  expect_identical(topn(x0, 8L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:8], info="topn-0001.040")


  expect_identical(topn(x0, 9L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:9], info="topn-0001.041")


  expect_identical(topn(x0, 10L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:10], info="topn-0001.042")


  expect_identical(topn(x0, 11L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:11], info="topn-0001.043")


  expect_identical(topn(x0, 12L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:12], info="topn-0001.044")


  expect_identical(topn(x0, 13L, decreasing=TRUE), order(x0, decreasing=TRUE)[1:13], info="topn-0001.045")


  expect_identical(topn(x1, 1L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:1], info="topn-0001.046")


  expect_identical(topn(x1, 2L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:2], info="topn-0001.047")


  expect_identical(topn(x1, 3L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:3], info="topn-0001.048")


  expect_identical(topn(x1, 4L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:4], info="topn-0001.049")


  expect_identical(topn(x1, 5L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:5], info="topn-0001.050")


  expect_identical(topn(x1, 6L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:6], info="topn-0001.051")


  expect_identical(topn(x1, 7L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:7], info="topn-0001.052")


  expect_identical(topn(x1, 8L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:8], info="topn-0001.053")


  expect_identical(topn(x1, 9L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:9], info="topn-0001.054")


  expect_identical(topn(x1, 10L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:10], info="topn-0001.055")


  expect_identical(topn(x1, 11L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:11], info="topn-0001.056")


  expect_identical(topn(x1, 12L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:12], info="topn-0001.057")


  expect_identical(topn(x1, 13L, decreasing=TRUE), order(x1, decreasing=TRUE)[1:13], info="topn-0001.058")


  expect_identical(topn(x2, 1L, decreasing=TRUE), order(x2, decreasing=TRUE)[1:1], info="topn-0001.060")


  expect_identical(topn(x2, 2L, decreasing=TRUE), order(x2, decreasing=TRUE)[1:2], info="topn-0001.061")


  expect_identical(topn(x2, 3L, decreasing=TRUE), order(x2, decreasing=TRUE)[1:3], info="topn-0001.062")


  expect_identical(topn(x3, 1L, decreasing=TRUE), order(x3, decreasing=TRUE)[1:1], info="topn-0001.063")


  expect_identical(topn(x3, 2L, decreasing=TRUE), order(x3, decreasing=TRUE)[1:2], info="topn-0001.064")


  expect_identical(topn(x3, 3L, decreasing=TRUE), order(x3, decreasing=TRUE)[1:3], info="topn-0001.065")


  expect_error(topn(x0, -1L), pattern = "Please enter a positive integer larger or equal to 1.", fixed = TRUE, info="topn-0001.066")


  expect_identical(topn(x5, 2001L,decreasing = TRUE), order(x5, decreasing=TRUE)[1:2001], info="topn-0001.067")


  expect_warning(res <- topn(x0, 100L,decreasing = FALSE), pattern = "'n' is larger than length of 'vec'. 'n' will be set to length of 'vec'.", fixed = TRUE, info="topn-0001.068")
  expect_identical(res, order(x0)[1:13], info="topn-0001.068")


  expect_error(topn(x0, 10L, decreasing = NA), pattern = "Argument 'decreasing' must be TRUE or FALSE and length 1.", fixed = TRUE, info="topn-0001.069")


  expect_error(topn(s1, 10L, decreasing = NA), pattern = "S4 class objects are not supported.", fixed = TRUE, info="topn-0001.070")


  expect_error(topn(x4, 2L), pattern = "Type raw is not supported.", fixed = TRUE, info="topn-0001.071")


  expect_error(topn(x4, 2L, decreasing = TRUE), pattern = "Type raw is not supported.", fixed = TRUE, info="topn-0001.072")


  expect_error(topn(x4, 2L, decreasing=FALSE), pattern = "Type raw is not supported.", fixed = TRUE, info="topn-0001.073")


  expect_identical(topn(x5, 1L, decreasing=FALSE,hasna=FALSE), order(x5)[1:1], info="topn-0001.074")


  expect_identical(topn(x5, 2L, decreasing=FALSE,hasna=FALSE), order(x5)[1:2], info="topn-0001.075")


  expect_identical(topn(x5, 3L, decreasing=FALSE,hasna=FALSE), order(x5)[1:3], info="topn-0001.076")


  expect_identical(topn(x5, 4L, decreasing=FALSE,hasna=FALSE), order(x5)[1:4], info="topn-0001.077")


  expect_identical(topn(x5, 5L, decreasing=FALSE,hasna=FALSE), order(x5)[1:5], info="topn-0001.078")


  expect_identical(topn(x5, 6L, decreasing=FALSE,hasna=FALSE), order(x5)[1:6], info="topn-0001.079")


  expect_identical(topn(x5, 7L, decreasing=FALSE,hasna=FALSE), order(x5)[1:7], info="topn-0001.080")


  expect_identical(topn(x5, 8L, decreasing=FALSE,hasna=FALSE), order(x5)[1:8], info="topn-0001.081")


  expect_identical(topn(x5, 9L, decreasing=FALSE,hasna=FALSE), order(x5)[1:9], info="topn-0001.082")


  expect_identical(topn(x5, 10L, decreasing=FALSE,hasna=FALSE), order(x5)[1:10], info="topn-0001.083")


  expect_identical(topn(x5, 11L, decreasing=FALSE,hasna=FALSE), order(x5)[1:11], info="topn-0001.084")


  expect_identical(topn(x5, 12L, decreasing=FALSE,hasna=FALSE), order(x5)[1:12], info="topn-0001.085")


  expect_identical(topn(x5, 13L, decreasing=FALSE,hasna=FALSE), order(x5)[1:13], info="topn-0001.086")


  expect_identical(topn(x5, 1L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:1], info="topn-0001.087")


  expect_identical(topn(x5, 2L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:2], info="topn-0001.088")


  expect_identical(topn(x5, 3L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:3], info="topn-0001.089")


  expect_identical(topn(x5, 4L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:4], info="topn-0001.090")


  expect_identical(topn(x5, 5L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:5], info="topn-0001.091")


  expect_identical(topn(x5, 6L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:6], info="topn-0001.092")


  expect_identical(topn(x5, 7L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:7], info="topn-0001.093")


  expect_identical(topn(x5, 8L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:8], info="topn-0001.094")


  expect_identical(topn(x5, 9L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:9], info="topn-0001.095")


  expect_identical(topn(x5, 10L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:10], info="topn-0001.096")


  expect_identical(topn(x5, 11L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:11], info="topn-0001.097")


  expect_identical(topn(x5, 12L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:12], info="topn-0001.098")


  expect_identical(topn(x5, 13L, decreasing=TRUE,hasna=FALSE), order(x5,decreasing = TRUE)[1:13], info="topn-0001.099")


  expect_identical(topn(x6, 1L, decreasing=FALSE,hasna=FALSE), order(x6)[1:1], info="topn-0001.100")


  expect_identical(topn(x6, 2L, decreasing=FALSE,hasna=FALSE), order(x6)[1:2], info="topn-0001.101")


  expect_identical(topn(x6, 3L, decreasing=FALSE,hasna=FALSE), order(x6)[1:3], info="topn-0001.102")


  expect_identical(topn(x6, 4L, decreasing=FALSE,hasna=FALSE), order(x6)[1:4], info="topn-0001.103")


  expect_identical(topn(x6, 5L, decreasing=FALSE,hasna=FALSE), order(x6)[1:5], info="topn-0001.104")


  expect_identical(topn(x6, 6L, decreasing=FALSE,hasna=FALSE), order(x6)[1:6], info="topn-0001.105")


  expect_identical(topn(x6, 7L, decreasing=FALSE,hasna=FALSE), order(x6)[1:7], info="topn-0001.106")


  expect_identical(topn(x6, 8L, decreasing=FALSE,hasna=FALSE), order(x6)[1:8], info="topn-0001.107")


  expect_identical(topn(x6, 9L, decreasing=FALSE,hasna=FALSE), order(x6)[1:9], info="topn-0001.108")


  expect_identical(topn(x6, 10L, decreasing=FALSE,hasna=FALSE), order(x6)[1:10], info="topn-0001.109")


  expect_identical(topn(x6, 11L, decreasing=FALSE,hasna=FALSE), order(x6)[1:11], info="topn-0001.110")


  expect_identical(topn(x6, 12L, decreasing=FALSE,hasna=FALSE), order(x6)[1:12], info="topn-0001.111")


  expect_identical(topn(x6, 13L, decreasing=FALSE,hasna=FALSE), order(x6)[1:13], info="topn-0001.112")


  expect_identical(topn(x6, 1L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:1], info="topn-0001.113")


  expect_identical(topn(x6, 2L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:2], info="topn-0001.114")


  expect_identical(topn(x6, 3L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:3], info="topn-0001.115")


  expect_identical(topn(x6, 4L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:4], info="topn-0001.116")


  expect_identical(topn(x6, 5L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:5], info="topn-0001.117")


  expect_identical(topn(x6, 6L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:6], info="topn-0001.118")


  expect_identical(topn(x6, 7L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:7], info="topn-0001.119")


  expect_identical(topn(x6, 8L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:8], info="topn-0001.120")


  expect_identical(topn(x6, 9L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:9], info="topn-0001.121")


  expect_identical(topn(x6, 10L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:10], info="topn-0001.122")


  expect_identical(topn(x6, 11L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:11], info="topn-0001.123")


  expect_identical(topn(x6, 12L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:12], info="topn-0001.124")


  expect_identical(topn(x6, 13L, decreasing=TRUE,hasna=FALSE), order(x6,decreasing = TRUE)[1:13], info="topn-0001.125")


  expect_error(topn(x4, 2L, decreasing = TRUE,hasna = FALSE), pattern = "Type raw is not supported.", fixed = TRUE, info="topn-0001.126")


  expect_error(topn(x4, 2L, decreasing=FALSE,hasna = FALSE), pattern = "Type raw is not supported.", fixed = TRUE, info="topn-0001.127")


  expect_error(topn(c(1,2,4,10,2,3), 2L, hasna=c(FALSE,TRUE)), pattern = "Argument 'hasna' must be TRUE or FALSE and length 1.", fixed = TRUE, info="topn-0001.128")


  expect_identical(topn(x5, 2001L,decreasing = FALSE), order(x5, decreasing=FALSE)[1:2001], info="topn-0001.129")


  expect_identical(topn(x7,1e4,decreasing=FALSE), order(x7, decreasing=FALSE), info="topn-0001.130")


  expect_identical(topn(as.numeric(x7),1e4,decreasing=FALSE), order(as.numeric(x7), decreasing=FALSE), info="topn-0001.131")

# issue #74: internal order() must resolve in base, not in the caller's frame.
local({
  order <- function(...) seq_along(..1)
  expect_identical(topn(seq_len(3000), 2001L, decreasing = TRUE), 3000:1000,
                   info = "topn-0074.001 caller-defined order ignored")
})

# issue #74: adversarial ordered input must fall back to the radix order() path.
adv <- seq_len(1e5)
expect_identical(topn(adv, 1500L, decreasing = TRUE, hasna = FALSE),
                 order(adv, decreasing = TRUE)[1:1500],
                 info = "topn-0074.002 fallback n=1500")
expect_identical(topn(adv, 1501L, decreasing = TRUE, hasna = FALSE)[1:1500],
                 topn(adv, 1500L, decreasing = TRUE, hasna = FALSE),
                 info = "topn-0074.003 fallback/radix boundary")
expect_identical(typeof(topn(adv, 1500L, decreasing = TRUE, hasna = FALSE)),
                 "integer", info = "topn-0074.004 fallback output type")

dbl <- c(seq_len(1e5) + 0.5, NA_real_, NaN)
expect_identical(topn(dbl, 1500L, decreasing = TRUE, hasna = TRUE),
                 order(dbl, decreasing = TRUE, na.last = TRUE)[1:1500],
                 info = "topn-0074.005 fallback doubles with NA/NaN")

rev_adv <- rev(seq_len(1e5))
expect_identical(topn(rev_adv, 2000L, decreasing = FALSE, hasna = FALSE),
                 order(rev_adv, decreasing = FALSE)[1:2000],
                 info = "topn-0074.006 fallback ascending")

ties <- rep(c(2L, 1L, 2L, NA_integer_), length.out = 1e5)
expect_identical(topn(ties, 1500L, decreasing = TRUE, hasna = TRUE),
                 order(ties, decreasing = TRUE, na.last = TRUE)[1:1500],
                 info = "topn-0074.007 fallback ties/NA")

# Worst case must not stay O(len1 * len0): 4e6 monotone elements take ~3 s
# with the unbounded scan and well under 1 s with the radix fallback.
t0 <- proc.time()[["elapsed"]]
bounded <- topn(seq_len(4e6), 1500L, decreasing = TRUE, hasna = FALSE)
elapsed <- proc.time()[["elapsed"]] - t0
expect_identical(bounded, 4e6:3998501, info = "topn-0074.008 bounded adversarial result")
expect_true(elapsed < 1.5, info = "topn-0074.009 bounded adversarial time")

