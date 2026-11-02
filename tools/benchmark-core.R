# Manual benchmark for the issue #107 hot paths. Not part of the test suite.
#
# Usage: Rscript tools/benchmark-core.R [library] [label] [reps]
#   library  optional library path to load kit from (e.g. a baseline install)
#   label    free-form label printed in every row (default "current")
#   reps     timed repetitions per case (default 5)
#
# Covers 2m/10m elements, 5/20/100 switch values, integer/string/factor/NA
# inputs, data-frame funique/uniqLen, and 1/2/4 threads on the 100-value cases.
# data.table is optional; its reference cases are skipped when unavailable.

args <- commandArgs(trailingOnly = TRUE)
if (length(args) > 3L) stop("Usage: Rscript tools/benchmark-core.R [library] [label] [reps]")
if (length(args) >= 1L && nzchar(args[[1]])) .libPaths(c(args[[1]], .libPaths()))
label <- if (length(args) >= 2L) args[[2]] else "current"
reps <- if (length(args) >= 3L) as.integer(args[[3]]) else 5L
library(kit)
has_dt <- requireNamespace("data.table", quietly = TRUE)

bench <- function(f) {
  invisible(f())
  times <- numeric(reps)
  for (i in seq_len(reps)) {
    invisible(gc(FALSE))
    times[[i]] <- system.time(f())[["elapsed"]]
  }
  median(times)
}

row <- function(case, n, m = NA_integer_, threads = 1L, f) {
  cat(sprintf("%s\t%s\t%s\t%d\t%d\t%d\t%.6f\n",
              label, case, R.version$platform, n, m, threads, bench(f)))
}

set.seed(107)
n10 <- 10000000L
base_int <- sample.int(1000L, n10, TRUE)
base_str <- paste0("v", base_int)
base_na <- base_str
base_na[sample.int(n10, n10 %/% 20L)] <- NA_character_
base_fac <- factor(base_str, levels = paste0("v", 1:1000))
vals_int <- sample.int(1000L, 100L)
vals_str <- paste0("v", vals_int)
vals_na <- vals_str; vals_na[100L] <- NA_character_
vals_fac <- factor(vals_str, levels = levels(base_fac))

cat(sprintf("# R\t%s\tplatform\t%s\tpackage\t%s\tlabel\t%s\treps\t%d\n",
            R.version.string, R.version$platform,
            as.character(packageVersion("kit")), label, reps))

for (n in c(2000000L, n10)) {
  xi <- base_int[seq_len(n)]
  xs <- base_str[seq_len(n)]
  xn <- base_na[seq_len(n)]
  xf <- base_fac[seq_len(n)]
  for (m in c(5L, 20L, 100L)) {
    out <- seq_len(m)
    vi <- vals_int[seq_len(m)]
    vs <- vals_str[seq_len(m)]
    vn <- vals_na[seq_len(m)]
    vf <- vals_fac[seq_len(m)]
    stopifnot(length(vswitch(xi, vi, out)) == n,
              length(vswitch(xs, vs, out)) == n,
              length(vswitch(xn, vn, out)) == n,
              length(vswitch(xf, vf, out)) == n)
    row("vswitch_int", n, m, f = function() vswitch(xi, vi, out))
    row("vswitch_str", n, m, f = function() vswitch(xs, vs, out))
    row("vswitch_na", n, m, f = function() vswitch(xn, vn, out))
    row("vswitch_factor", n, m, f = function() vswitch(xf, vf, out))
    pairs_i <- unlist(lapply(seq_len(m), function(i) list(vi[i], i)), recursive = FALSE)
    pairs_s <- unlist(lapply(seq_len(m), function(i) list(vs[i], i)), recursive = FALSE)
    row("nswitch_int", n, m, f = function() {
      do.call(nswitch, c(list(xi), pairs_i, list(default = 0L)))
    })
    row("nswitch_str", n, m, f = function() {
      do.call(nswitch, c(list(xs), pairs_s, list(default = 0L)))
    })
    if (has_dt && m == 100L) {
      row("ref_chmatch_str", n, m, f = function() data.table::chmatch(xs, vs))
    }
  }
}

for (n in c(2000000L, n10)) {
  set.seed(107)
  xi <- sample.int(100000L, n, TRUE)
  xs <- sprintf("id%06d", sample.int(100000L, n, TRUE))
  xseq <- rep(seq_len(100000L), length.out = n)
  df_int <- data.frame(a = xi, b = sample.int(1000L, n, TRUE))
  df_mix <- data.frame(a = xi, b = rnorm(n))
  row("funique_int", n, f = function() funique(xi))
  row("funique_seq", n, f = function() funique(xseq))
  row("funique_str", n, f = function() funique(xs))
  row("uniqLen_int", n, f = function() uniqLen(xi))
  row("uniqLen_df_int", n, f = function() uniqLen(df_int))
  row("funique_df_int", n, f = function() funique(df_int))
  row("uniqLen_df_mixed", n, f = function() uniqLen(df_mix))
  row("funique_df_mixed", n, f = function() funique(df_mix))
  if (has_dt) {
    row("ref_uniqueN_int", n, f = function() data.table::uniqueN(xi))
    row("ref_uniqueN_df_int", n, f = function() data.table::uniqueN(df_int))
    row("ref_uniqueN_df_mixed", n, f = function() data.table::uniqueN(df_mix))
  }
}

# Thread scaling on the 100-value cases.
for (n in c(2000000L, n10)) {
  xi <- base_int[seq_len(n)]
  xs <- base_str[seq_len(n)]
  pairs_i <- unlist(lapply(1:100, function(i) list(vals_int[i], i)), recursive = FALSE)
  pairs_s <- unlist(lapply(1:100, function(i) list(vals_str[i], i)), recursive = FALSE)
  for (th in c(1L, 2L, 4L)) {
    row("vswitch_int_100t", n, 100L, th,
        function() vswitch(xi, vals_int, 1:100, nThread = th))
    row("vswitch_str_100t", n, 100L, th,
        function() vswitch(xs, vals_str, 1:100, nThread = th))
    row("nswitch_int_100t", n, 100L, th, function() {
      do.call(nswitch, c(list(xi), pairs_i, list(default = 0L, nThread = th)))
    })
    row("nswitch_str_100t", n, 100L, th, function() {
      do.call(nswitch, c(list(xs), pairs_s, list(default = 0L, nThread = th)))
    })
  }
}
