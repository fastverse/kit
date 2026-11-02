# Tests for the hash lookup used by vswitch()/nswitch() with many values (#107).
# The hash path is selected for more than 8 value/output pairs; these cases pin
# down the matching semantics it must share with the value-major fallback:
# last pair wins, NA/NaN handling per type, encoding, vector recycling, and
# thread determinism.

sys.source("helper-kit.R", envir = environment())

# Reference for types where NA matches NA (integer, logical, string, factor).
ref_na <- function(x, values, outputs, default = NA) {
  ans <- rep(default, length(x))
  for (i in seq_along(values)) {
    hit <- if (is.na(values[i])) is.na(x) else !is.na(x) & x == values[i]
    ans[hit] <- rep_len(outputs[[i]], length(x))[hit]
  }
  ans
}

# Reference for doubles/complex, where NA and NaN never match.
ref_num <- function(x, values, outputs, default = NA) {
  ans <- rep(default, length(x))
  for (i in seq_along(values)) {
    hit <- !is.na(values[i]) & !is.nan(values[i]) &
      !is.na(x) & !is.nan(x) & x == values[i]
    ans[hit] <- rep_len(outputs[[i]], length(x))[hit]
  }
  ans
}

# Integer: NA is the same bit pattern, so it matches NA.
vals_i <- c(9:1, 1L, NA_integer_)
outs_i <- c(101:109, 110L, 111L)
x_i <- c(1:9, NA_integer_, NA_integer_, 0L)
exp_i <- ref_na(x_i, vals_i, as.list(outs_i))
expect_identical(vswitch(x_i, vals_i, outs_i), exp_i,
                 info = "hash-vswitch-int")
expect_identical(vswitch(x_i, vals_i, outs_i, nThread = 2L), exp_i,
                 info = "hash-vswitch-int-threads")

vals_l <- c(rep(TRUE, 9), FALSE, NA)
outs_l <- c(1:9, 10L, 11L)
x_l <- c(NA, TRUE, FALSE, TRUE)
expect_identical(vswitch(x_l, vals_l, outs_l),
                 ref_na(x_l, vals_l, as.list(outs_l)),
                 info = "hash-vswitch-lgl")

# Strings: NA_STRING is a CHARSXP and therefore matches itself.
vals_s <- c(paste0("v", 1:9), "v1", NA_character_)
outs_s <- c(1:9, 10L, 11L)
x_s <- c("v1", "v9", NA_character_, "zz")
exp_s <- ref_na(x_s, vals_s, as.list(outs_s))
expect_identical(vswitch(x_s, vals_s, outs_s), exp_s,
                 info = "hash-vswitch-str")
expect_identical(vswitch(enc2utf8(x_s), enc2utf8(vals_s), outs_s), exp_s,
                 info = "hash-vswitch-str-utf8")

enc_lat <- "fa\xE7ile"
Encoding(enc_lat) <- "latin1"
enc_utf <- enc2utf8(enc_lat)
vals_e <- c(paste0("v", 1:8), enc_lat, enc_utf)
x_e <- c(enc_lat, enc_utf, paste0("v", 1:8))
expect_identical(vswitch(x_e, vals_e, c(1:8, 9L, 10L)),
                 c(10L, 10L, 1:8), info = "hash-vswitch-str-enc")

# Doubles: -0 == 0, Inf matches, but NA/NaN never match.
vals_d <- c(1:8, Inf, 0, NA_real_, NaN)
outs_d <- c(101:108, 109L, 110L, 111L, 112L)
x_d <- c(-0, 0, Inf, NA_real_, NaN, 1:8, 99)
exp_d <- ref_num(x_d, vals_d, as.list(outs_d))
expect_identical(vswitch(x_d, vals_d, outs_d), exp_d,
                 info = "hash-vswitch-dbl")
expect_identical(vswitch(x_d, vals_d, outs_d, nThread = 2L), exp_d,
                 info = "hash-vswitch-dbl-threads")

# Complex: component-wise ==, so NA/NaN components never match.
vals_c <- c(as.complex(1:8), 0 + 0i, NA_complex_)
outs_c <- c(101:108, 110L, 111L)
x_c <- c(0 + 0i, 1 + 0i, NA_complex_, 1 + 1i)
expect_identical(vswitch(x_c, vals_c, outs_c),
                 ref_num(x_c, vals_c, as.list(outs_c)),
                 info = "hash-vswitch-cplx")

# Factor codes go through the integer path; classes and levels are preserved.
f_x <- factor(c(letters[1:9], "a", NA), levels = letters[1:9])
f_v <- factor(c(letters[1:9], "a", NA), levels = letters[1:9])
f_o <- factor(c(letters[9:1], "a", NA), levels = letters[1:9])
ans_f <- vswitch(f_x, f_v, f_o, factor(NA, levels = letters[1:9]))
expect_identical(ans_f,
                 factor(c("a", letters[8:1], "a", NA), levels = letters[1:9]),
                 info = "hash-vswitch-factor")

# Date outputs keep their class.
d_o <- as.Date("2020-01-01") + 0:10
exp_dt <- as.Date("2020-01-01") + c(9, 7:0, 10, 10, NA)
expect_identical(vswitch(x_i, vals_i, as.list(d_o), as.Date(NA)), exp_dt,
                 info = "hash-vswitch-date")

# Element-length outputs are recycled by x position, not by pair.
out_vec <- lapply(seq_along(vals_i), function(i) seq_along(x_i) + i * 100L)
exp_vec <- ref_na(x_i, vals_i, out_vec)
expect_identical(vswitch(x_i, vals_i, out_vec), exp_vec,
                 info = "hash-vswitch-vector-output")

# Empty input still accepts many values and returns an empty result.
expect_identical(vswitch(integer(), vals_i, outs_i), integer(),
                 info = "hash-vswitch-empty")

# nswitch shares the same lookup. Duplicate values keep the last pair.
pairs <- unlist(lapply(1:9, function(i) list(i, i + 100L)), recursive = FALSE)
pairs <- c(pairs, list(1L, 999L, NA_integer_, 998L))
ns <- function(x) do.call(nswitch, c(list(x), pairs, list(default = 0L)))
expect_identical(ns(c(1:9, NA_integer_, NA_integer_, 0L)),
                 c(999L, 102:109, 998L, 998L, 0L),
                 info = "hash-nswitch-int")
expect_identical(do.call(nswitch, c(list(c(1:9, NA_integer_)), pairs,
                                    list(default = 0L, nThread = 2L))),
                 ns(c(1:9, NA_integer_)), info = "hash-nswitch-threads")

pairs_s <- unlist(lapply(1:9, function(i) list(paste0("v", i), i + 100L)),
                   recursive = FALSE)
pairs_s <- c(pairs_s, list("v1", 999L, NA_character_, 998L))
ns_s <- function(x) do.call(nswitch, c(list(x), pairs_s, list(default = 0L)))
expect_identical(ns_s(c("v1", "v9", NA_character_, "zz")),
                 c(999L, 109L, 998L, 0L), info = "hash-nswitch-str")

# nswitch vector outputs are recycled by element of x.
pairs_v <- unlist(lapply(seq_along(vals_i),
                         function(i) list(vals_i[i], seq_along(x_i) + i * 100L)),
                  recursive = FALSE)
ns_v <- function(x) do.call(nswitch, c(list(x), pairs_v,
                                       list(default = NA_integer_)))
expect_identical(ns_v(x_i), exp_vec, info = "hash-nswitch-vector-output")

# list inputs stay on the existing deep-equality path.
xl <- as.list(x_i)
expect_identical(vswitch(xl, as.list(vals_i), outs_i), exp_i,
                 info = "hash-vswitch-list-input")
