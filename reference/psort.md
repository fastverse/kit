# Parallel Sort

Similar to [`base::sort`](https://rdrr.io/r/base/sort.html), but only
for character vectors and partly using parallelism. It is experimental
and might change in the future. Use with caution.

## Usage

``` r
psort(x, decreasing=FALSE, na.last=NA,
      nThread=getOption("kit.nThread"),c.locale=TRUE)
```

## Arguments

- x:

  A character vector. For other input, `psort()` warns and calls
  [`base::sort()`](https://rdrr.io/r/base/sort.html).

- na.last:

  Controls the treatment of `NA`s. If `TRUE`, missing values are put
  last; if `FALSE`, they are put first; if `NA`, they are removed.

- decreasing:

  Logical. If `TRUE`, sort in decreasing order. Default is `FALSE`.

- nThread:

  Integer. Number of threads to use. Default is
  `getOption("kit.nThread")`.

- c.locale:

  Logical. If `TRUE` (default), sort byte by byte in the C locale. The
  result is the same on every operating system and matches
  `sort(x, method = "radix")`. If `FALSE`, use the session locale and
  `sort(x, method = "quick")`; the order of accented letters and other
  non-ASCII text can differ between operating systems and locales.

## Details

For non-character `x`, `psort()` warns and calls
[`base::sort()`](https://rdrr.io/r/base/sort.html) with
`method = "radix"` when `c.locale = TRUE`, and `method = "quick"`
otherwise.

## Value

A character vector with the values of `x` in sorted order.

## Author

Morgan Jacob

## Examples

``` r
x = c("b","A","B","a","\xe4")
Encoding(x) = "latin1"
identical(psort(x, c.locale=FALSE), sort(x))
#> [1] TRUE
identical(psort(x, c.locale=TRUE), sort(x, method="radix"))
#> [1] TRUE

# Benchmarks
# ----------
# strings = as.character(as.hexmode(1:1000))
# x = sample(strings, 1e8, replace=TRUE)
# system.time({kit::psort(x, na.last = TRUE, nThread = 1L)})
#   user  system elapsed
#  2.833   0.434   3.277
# system.time({sort(x,method="radix",na.last = TRUE)})
#   user  system elapsed
#  5.597   0.559   6.176
# system.time({x[order(x,method="radix",na.last = TRUE)]})
#   user  system elapsed
#  5.561   0.563   6.143
```
