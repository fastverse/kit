# kit

[![CRAN](https://www.r-pkg.org/badges/version-last-release/kit?color=blue)](https://cran.r-project.org/package=kit)
[![CRAN](https://badges.cranchecks.info/flavor/release/kit.svg)](https://cran.r-project.org/web/checks/check_results_kit.html)
[![License: GPL v3](https://img.shields.io/github/license/fastverse/kit)](https://www.gnu.org/licenses/gpl-3.0)
[![R-CMD-check.yaml](https://github.com/fastverse/kit/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/fastverse/kit/actions/workflows/R-CMD-check.yaml)
[![Coverage Status](https://codecov.io/gh/fastverse/kit/graph/badge.svg)](https://codecov.io/github/fastverse/kit?branch=master)
[![downloads](https://cranlogs.r-pkg.org/badges/kit)](https://www.r-pkg.org/pkg/kit)
[![kit status badge](https://fastverse.r-universe.dev/badges/kit)](https://fastverse.r-universe.dev)

Fast data manipulation functions implemented in C for large datasets. Provides vectorized alternatives to base R functions with significant performance improvements.

## Installation

```r
# From CRAN
install.packages("kit")

# Development version
install.packages("kit", repos = "https://fastverse.r-universe.dev")
```

## Features

### Parallel Statistical Functions

Vector-valued functions operating in parallel over vectors or data frames:

- **`psum`, `pprod`, `pmean`**: Parallel sum, product, and mean
- **`fpmin`, `fpmax`, `prange`**: Parallel minimum, maximum, and range (complements base `pmin`/`pmax` with `na.rm` support)
- **`pall`, `pany`**: Parallel all/any operations
- **`pcount`, `pcountNA`**: Count occurrences of values or NAs
- **`pfirst`, `plast`**: First/last non-missing values

```r
x <- c(1, 3, NA, 5)
y <- c(2, NA, 4, 1)
psum(x, y, na.rm = TRUE)  # [1] 3 3 4 6
pmean(x, y, na.rm = TRUE) # [1] 1.5 3.0 4.0 3.0
fpmin(x, y, na.rm = TRUE) # [1] 1 3 4 1
fpmax(x, y, na.rm = TRUE) # [1] 2 3 4 5
prange(x, y, na.rm = TRUE) # [1] 1 0 0 4
```

### Vectorized and Nested Switches

Fast vectorized conditional logic:

- **`iif`**: Fast replacement for `ifelse()` with attribute preservation
- **`nif`**: Nested if-else (SQL CASE WHEN equivalent)
- **`vswitch`, `nswitch`**: Vectorized switch statements

```r
iif(x > 2, x, x - 1)  # Preserves attributes unlike base::ifelse
nif(x == 1, "one", x == 2, "two", default = "other")
```

### Sorting

- **`psort`**: Parallel sort for character vectors
- **`topn`**: Efficient partial sort (top N values) without full sorting

```r
topn(x, n = 6L, decreasing = TRUE)  # Much faster than order()[1:6]
```

### Factors

- **`charToFact`**: Fast character-to-factor conversion
- **`setlevels`**: Change factor levels by reference

### Unique Values and Counts

- **`funique`, `fduplicated`**: Fast unique/duplicated operations
- **`uniqLen`**: Fast equivalent to `length(unique(x))`
- **`count`, `countNA`, `countOccur`**: Count element occurrences

```r
funique(iris$Species)  # Faster than base::unique
uniqLen(iris$Species)  # Faster than length(unique())
```

### Miscellaneous

- **`fpos`**: Find matrix/vector positions within larger structures
- **`shareData`, `getData`, `clearData`**: Share data between R sessions

## Documentation

Full documentation available at: https://fastverse.org/kit/

## License

GPL-3
