args <- commandArgs(trailingOnly = TRUE)
if (length(args) > 2L) stop("Usage: Rscript tools/benchmark-psum.R [library] [label]")
if (length(args) >= 1L && nzchar(args[[1]])) .libPaths(c(args[[1]], .libPaths()))
label <- if (length(args) >= 2L) args[[2]] else "current"
library(kit)
options(kit.nThread = 1L)

bench <- function(f, reps = 21L, calls = 5L) {
  invisible(f())
  times <- numeric(reps)
  for (i in seq_len(reps)) {
    invisible(gc())
    start <- proc.time()[["elapsed"]]
    for (j in seq_len(calls)) f()
    times[[i]] <- (proc.time()[["elapsed"]] - start) / calls
  }
  unname(times)
}

summarize <- function(name, f) {
  times <- bench(f)
  quartiles <- unname(quantile(times, probs = c(0.25, 0.5, 0.75), names = FALSE))
  cat(sprintf("%s\t%s\tmedian\t%.6f\tq1\t%.6f\tq3\t%.6f\tmin\t%.6f\n", label, name, quartiles[[2]], quartiles[[1]], quartiles[[3]], min(times)))
}

set.seed(74L)
n <- 10000000L
x <- rnorm(n)
y <- rnorm(n)
z <- rnorm(n)
xn <- x
yn <- y
zn <- z
xn[seq.int(20L, n, by = 20L)] <- NA_real_
yn[seq.int(40L, n, by = 40L)] <- NA_real_
zn[seq.int(60L, n, by = 60L)] <- NA_real_
xi <- sample.int(1000L, n, replace = TRUE)
yi <- sample.int(1000L, n, replace = TRUE)
zi <- sample.int(1000L, n, replace = TRUE)
stopifnot(identical(psum(xi, yi, zi, na.rm = TRUE), xi + yi + zi))

cat(sprintf("# R\t%s\tplatform\t%s\tpackage\t%s\tlabel\t%s\n", R.version.string, R.version$platform, as.character(packageVersion("kit")), label))
summarize("double3_clean", function() psum(x, y, z, na.rm = TRUE))
summarize("double3_na5pct", function() psum(xn, yn, zn, na.rm = TRUE))
summarize("integer3_clean", function() psum(xi, yi, zi, na.rm = TRUE))
summarize("double2_clean", function() psum(x, y, na.rm = TRUE))
summarize("double3_narm_false", function() psum(x, y, z, na.rm = FALSE))

rm(x, y, z, xn, yn, zn, xi, yi, zi)
invisible(gc())
ncomplex <- 5000000L
cx <- complex(real = rnorm(ncomplex), imaginary = rnorm(ncomplex))
cy <- complex(real = rnorm(ncomplex), imaginary = rnorm(ncomplex))
cz <- complex(real = rnorm(ncomplex), imaginary = rnorm(ncomplex))
summarize("complex3_clean", function() psum(cx, cy, cz, na.rm = TRUE))
rm(cx, cy, cz)
invisible(gc())
n8 <- 5000000L
d1 <- rnorm(n8)
d2 <- rnorm(n8)
d3 <- rnorm(n8)
d4 <- rnorm(n8)
d5 <- rnorm(n8)
d6 <- rnorm(n8)
d7 <- rnorm(n8)
d8 <- rnorm(n8)
summarize("double8_clean", function() psum(d1, d2, d3, d4, d5, d6, d7, d8, na.rm = TRUE))
