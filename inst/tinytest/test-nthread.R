# Determinism of kit.nThread = 1 vs 2 — tinytest (see issue #63).
# Covers iif / vswitch / nswitch / charToFact / psort: identical results
# irrespective of thread count, including NA, Date and factor payloads.
# Skipped when OpenMP is disabled (serial build): determinism holds trivially.

sys.source("helper-kit.R", envir = environment())

if (isFALSE(.Call(kit:::CompEnabledR))) {
  exit_file("OpenMP disabled: nThread determinism holds trivially")
}

old_nthread <- getOption("kit.nThread")
on.exit(options(kit.nThread = old_nthread), add = TRUE)
set.seed(123)

n <- 5000L
ti <- sample(c(TRUE, FALSE, NA), n, TRUE)
xi <- sample(1:5, n, TRUE)
xd <- rnorm(n)
xs <- sample(c(letters[1:5], NA), n, TRUE)
xf <- factor(sample(c("a", "b", "c", NA), n, TRUE))
xdate <- as.Date("2020-01-01") + sample(0:9, n, TRUE)

# --- iif ---
expect_identical(
  iif(ti, xi, -xi, nThread = 1L),
  iif(ti, xi, -xi, nThread = 2L),
  info="nthread-iif-001"
)
expect_identical(
  iif(ti, xd, -xd, nThread = 1L),
  iif(ti, xd, -xd, nThread = 2L),
  info="nthread-iif-002"
)
expect_identical(
  iif(ti, xs, "missing", nThread = 1L),
  iif(ti, xs, "missing", nThread = 2L),
  info="nthread-iif-003"
)
expect_identical(
  iif(ti, xf, factor(NA, levels = c("a", "b", "c")), nThread = 1L),
  iif(ti, xf, factor(NA, levels = c("a", "b", "c")), nThread = 2L),
  info="nthread-iif-004"
)
expect_identical(
  iif(ti, xdate, as.Date("2021-06-01"), nThread = 1L),
  iif(ti, xdate, as.Date("2021-06-01"), nThread = 2L),
  info="nthread-iif-005"
)
expect_identical(
  iif(ti, xi, -xi, NA_integer_, tprom = TRUE, nThread = 1L),
  iif(ti, xi, -xi, NA_integer_, tprom = TRUE, nThread = 2L),
  info="nthread-iif-006"
)

# --- vswitch ---
expect_identical(
  vswitch(xi, 1:5, as.list(11:15), nThread = 1L),
  vswitch(xi, 1:5, as.list(11:15), nThread = 2L),
  info="nthread-vswitch-001"
)
expect_identical(
  vswitch(xs, letters[1:5], as.list(LETTERS[1:5]), default = "?", nThread = 1L),
  vswitch(xs, letters[1:5], as.list(LETTERS[1:5]), default = "?", nThread = 2L),
  info="nthread-vswitch-002"
)
expect_identical(
  vswitch(xf, factor(c("a", "b", "c")), list(1L, 2L, 3L), nThread = 1L),
  vswitch(xf, factor(c("a", "b", "c")), list(1L, 2L, 3L), nThread = 2L),
  info="nthread-vswitch-003"
)

# --- nswitch ---
expect_identical(
  nswitch(xi, 1L, "a", 2L, "b", default = "z", nThread = 1L),
  nswitch(xi, 1L, "a", 2L, "b", default = "z", nThread = 2L),
  info="nthread-nswitch-001"
)
expect_identical(
  nswitch(xi, 1L, 10L, 2L, 20L, default = 0L, nThread = 1L),
  nswitch(xi, 1L, 10L, 2L, 20L, default = 0L, nThread = 2L),
  info="nthread-nswitch-002"
)
# duplicate values: last matching pair wins; the pair loop is serial so the
# result cannot depend on thread scheduling (issue #63 follow-up)
expect_identical(
  nswitch(rep(1L, n), 1L, 10L, 1L, 20L, default = 0L, nThread = 1L),
  nswitch(rep(1L, n), 1L, 10L, 1L, 20L, default = 0L, nThread = 2L),
  info="nthread-nswitch-dup-001"
)
expect_true(
  all(nswitch(rep(1L, n), 1L, 10L, 1L, 20L, default = 0L, nThread = 2L) == 20L),
  info="nthread-nswitch-dup-002"
)

# --- charToFact ---
expect_identical(
  charToFact(xs, nThread = 1L),
  charToFact(xs, nThread = 2L),
  info="nthread-charToFact-001"
)
expect_identical(
  charToFact(xs, decreasing = TRUE, nThread = 1L),
  charToFact(xs, decreasing = TRUE, nThread = 2L),
  info="nthread-charToFact-002"
)
expect_identical(
  charToFact(xs, addNA = FALSE, nThread = 1L),
  charToFact(xs, addNA = FALSE, nThread = 2L),
  info="nthread-charToFact-003"
)

# --- psort ---
expect_identical(
  psort(xs, nThread = 1L),
  psort(xs, nThread = 2L),
  info="nthread-psort-001"
)
expect_identical(
  psort(xs, decreasing = TRUE, nThread = 1L),
  psort(xs, decreasing = TRUE, nThread = 2L),
  info="nthread-psort-002"
)
expect_identical(
  psort(xs, na.last = FALSE, nThread = 1L),
  psort(xs, na.last = FALSE, nThread = 2L),
  info="nthread-psort-003"
)
expect_identical(
  psort(xs, c.locale = FALSE, nThread = 1L),
  psort(xs, c.locale = FALSE, nThread = 2L),
  info="nthread-psort-004"
)

# --- kit.nThread option is honoured as default ---
options(kit.nThread = 1L)
expect_identical(iif(ti, xi, -xi), iif(ti, xi, -xi, nThread = 1L), info="nthread-opt-001")
options(kit.nThread = 2L)
expect_identical(iif(ti, xi, -xi), iif(ti, xi, -xi, nThread = 2L), info="nthread-opt-002")
expect_identical(vswitch(xi, 1:5, as.list(11:15)), vswitch(xi, 1:5, as.list(11:15), nThread = 2L), info="nthread-opt-003")
