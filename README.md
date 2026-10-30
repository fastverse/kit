# kit

[![CRAN](https://www.r-pkg.org/badges/version-last-release/kit?color=blue)](https://cran.r-project.org/package=kit)
[![CRAN](https://badges.cranchecks.info/flavor/release/kit.svg)](https://cran.r-project.org/web/checks/check_results_kit.html)
[![License: GPL v3](https://img.shields.io/github/license/fastverse/kit)](https://www.gnu.org/licenses/gpl-3.0)
[![R-CMD-check.yaml](https://github.com/fastverse/kit/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/fastverse/kit/actions/workflows/R-CMD-check.yaml)
[![downloads](https://cranlogs.r-pkg.org/badges/kit)](https://www.r-pkg.org/pkg/kit)
[![kit status badge](https://fastverse.r-universe.dev/badges/kit)](https://fastverse.r-universe.dev)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fastverse/kit)

Fast data manipulation functions implemented in C for large datasets. Provides vectorized alternatives to base R functions with significant performance improvements.

## Installation

```r
# From CRAN
install.packages("kit")

# Development version
install.packages("kit", repos = "https://fastverse.r-universe.dev")
```

## OpenMP and threads

`kit` uses OpenMP in `iif()`, `nswitch()`, `vswitch()`, `psort()`, and
`charToFact()`. The other functions use one thread. OpenMP is optional.
Without OpenMP, `kit` still works, but these five functions use one thread.

- **Linux:** Install the OpenMP runtime for your compiler. With GCC, this is
  usually `libgomp` (`libgomp1` on Debian and Ubuntu). With Clang, install
  `libomp` (`libomp-dev` on Debian and Ubuntu).
- **macOS:** Apple Clang does not include OpenMP. Install it with Homebrew:
  `brew install libomp`. On Apple Silicon, Homebrew installs to a path that
  clang does not search by default. Add that path to your R Makevars before
  you install `kit` from source (`configure` decides with `CFLAGS`, so the
  include path must be there):

  ```sh
  # ~/.R/Makevars
  CFLAGS += -I$(shell brew --prefix libomp)/include -L$(shell brew --prefix libomp)/lib
  LDFLAGS += -L$(shell brew --prefix libomp)/lib
  ```

- **Windows:** OpenMP is included in Rtools. Install the Rtools version that
  matches your R version. Then install `kit` from source.

Set the number of threads once for each session:

```r
options(kit.nThread = 4L)
```

The default is `1`. If you set a value that is larger than the number of
available processors, `kit` uses the number of available processors. `kit`
reads the option each time you call a function, so you can change it between
calls.

To check whether OpenMP is enabled, run:

```r
.Call(kit:::CompEnabledR)
```

If this call returns `FALSE`, the five functions above use one thread, even
when `kit.nThread` is larger than `1`.

## Platforms

`kit` is tested on Linux, macOS, and Windows. OpenMP is optional. See
[OpenMP and threads](#openmp-and-threads) for the installation requirements.

`shareData()` behaves differently on POSIX systems and Windows:

- **Linux, macOS, and other POSIX systems:** `getData()` reads the object one
  time. A second call with the same name fails. If the sharing session stops
  before `clearData()`, the object can stay in shared memory: on Linux in
  `/dev/shm` until you remove it or restart the machine, on macOS until the
  machine restarts.
- **Windows:** `getData()` can read the object more than one time while the
  sharing session is alive. If the sharing session stops, Windows removes the
  mapping.

## Features

### Row-wise Statistical Functions

Vector-valued functions that combine several vectors or data frame columns element by element:

- **`psum`, `pprod`, `pmean`**: Row-wise sum, product, and mean
- **`fpmin`, `fpmax`, `prange`**: Row-wise minimum, maximum, and range
- **`pall`, `pany`**: Row-wise all/any operations
- **`pcount`, `pcountNA`**: Count occurrences of values or NAs
- **`pfirst`, `plast`**: First/last non-missing values

The word "parallel" in their names means they work across several vectors at once, not that they use multiple threads. These functions are single-threaded C code.

```r
library(kit)
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
