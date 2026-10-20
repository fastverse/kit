# Tests for charToFact() — fast character-to-factor conversion — tinytest (see issue #54).
# Migrated from testthat 3e to tinytest to keep kit lean (zero test dependencies).
# Each legacy check("id", actual, expected) maps to expect_identical(..., info="id");
# 31 tolerant cases (identical FALSE but all.equal+typeof TRUE) use expect_kit_equal(..., info="id");
# check(..., error=) maps to expect_error(pattern=, fixed=TRUE, info="id").
# Legacy IDs are preserved as info= labels (prefixed by function for uniqueness).
# Setup and expectations stay interleaved in legacy order: later sections may
# redefine x/y/out_vec/... so setup must not be hoisted above earlier tests.

sys.source("helper-kit.R", envir = environment())
set.seed(123)

x1 = sample(c(letters,LETTERS),1e2,TRUE)

  expect_identical(charToFact(c("a","b")), as.factor(c("a","b")), info="charToFact-0021.001")


  expect_identical(charToFact(c("a","b","a")), as.factor(c("a","b","a")), info="charToFact-0021.002")


  expect_identical(charToFact(x1), as.factor(x1), info="charToFact-0021.003")


  expect_error(charToFact(c("2L","1L","3L"),nThread=1), pattern = "Argument 'nThread' (double) must be of type integer.", fixed = TRUE, info="charToFact-0021.004")


  expect_error(charToFact(1L), pattern = "Argument 'x' must be of type character.", fixed = TRUE, info="charToFact-0021.005")


  expect_identical(charToFact(c("a","b",NA,"a")), addNA(as.factor(c("a","b",NA,"a"))), info="charToFact-0021.006")


  expect_identical(levels(charToFact(x1,decreasing = TRUE)), sort(levels(as.factor(x1)),decreasing = TRUE), info="levels-0021.007")


  expect_error(charToFact(c("a","b"),addNA=NA), pattern = "Argument 'addNA' must be TRUE or FALSE.", fixed = TRUE, info="charToFact-0021.008")


  expect_identical(charToFact(c("a","b",NA,"a"), addNA=FALSE), as.factor(c("a","b",NA,"a")), info="charToFact-0021.009")


  expect_identical(charToFact(c("a","b",NA,"c")), addNA(as.factor(c("a","b",NA,"c"))), info="charToFact-0021.010")


  expect_identical(charToFact(c("a","b",NA,"c"),addNA=FALSE), as.factor(c("a","b",NA,"c")), info="charToFact-0021.011")


  expect_identical(charToFact(c("a",NA,"b")), addNA(as.factor(c("a",NA,"b"))), info="charToFact-0021.012")


  expect_identical(charToFact(c("a",NA,"a","b")), addNA(as.factor(c("a",NA,"a","b"))), info="charToFact-0021.013")


  expect_identical(charToFact(c("a",NA,"aa","b")), addNA(as.factor(c("a",NA,"aa","b"))), info="charToFact-0021.014")


  expect_identical(charToFact(c("a",NA,"aa")), addNA(as.factor(c("a",NA,"aa"))), info="charToFact-0021.015")

# issue #74: internal sort() must resolve in base, not in the caller's frame.
local({
  sort <- function(x, ...) rev(x)
  expect_identical(charToFact(c("a", "b", "a")), as.factor(c("a", "b", "a")),
                   info = "charToFact-0074.001 caller-defined sort ignored")
})

# issue #74: the unique table grows past its initial 256 slots.
local({
  set.seed(74)
  x <- sample(paste0("s", 1:800), 5000L, TRUE)
  expect_identical(charToFact(x), as.factor(x), info = "charToFact-0074.grow")
  x <- sample(c(NA_character_, paste0("s", 1:800)), 5000L, TRUE)
  expect_identical(charToFact(x), addNA(as.factor(x)),
                   info = "charToFact-0074.grow-na")
})

