args <- commandArgs(trailingOnly = TRUE)
if (length(args) > 4L) stop("Usage: Rscript tools/benchmark-unique.R [library] [label] [n] [unique]")
if (length(args) >= 1L && nzchar(args[[1]])) .libPaths(c(args[[1]], .libPaths()))
label <- if (length(args) >= 2L) args[[2]] else "current"
n <- if (length(args) >= 3L) as.integer(args[[3]]) else 10000000L
u <- if (length(args) >= 4L) as.integer(args[[4]]) else 100000L
library(kit)
options(kit.nThread = 1L)

bench <- function(f, reps = 7L) {
  invisible(f())
  times <- numeric(reps)
  for (i in seq_len(reps)) {
    invisible(gc())
    times[[i]] <- system.time(f())[["elapsed"]]
  }
  median(times)
}

summarize <- function(name, f) {
  cat(sprintf("%s\t%s\t%.6f\n", label, name, bench(f)))
}

# Sequential IDs are the case that exposed the old top-bits-only hash: the
# table must still probe quickly when keys are contiguous, not just random.
x <- rep(seq_len(u), length.out = n)
stopifnot(identical(funique(x), seq_len(min(u, n))))

cat(sprintf("# R\t%s\tplatform\t%s\tpackage\t%s\tlabel\t%s\tn\t%d\tunique\t%d\n",
            R.version.string, R.version$platform,
            as.character(packageVersion("kit")), label, n, u))
summarize("funique_seq", function() funique(x))
summarize("uniqLen_seq", function() uniqLen(x))
summarize("fduplicated_seq", function() fduplicated(x))
