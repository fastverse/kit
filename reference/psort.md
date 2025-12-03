# Parallel Sort

Similar to [`base::sort`](https://rdrr.io/r/base/sort.html) but just for
character vector and partially using parallelism. It is currently
experimental and might change in the future. Use with caution.

## Usage

``` r
psort(x, decreasing=FALSE, na.last=NA,
      nThread=getOption("kit.nThread"),c.locale=TRUE)
```

## Arguments

- x:

  A vector of type character. If other, it will default to
  [`base::sort`](https://rdrr.io/r/base/sort.html)

- na.last:

  For controlling the treatment of `NA`s. If `TRUE`, missing values in
  the data are put last; if `FALSE`, they are put first; if `NA`, they
  are removed.

- decreasing:

  A boolean indicating where to sort the data in decreasing way. Default
  is `FALSE`.

- nThread:

  Number of thread to use. Default value is `1L`.

- c.locale:

  A boolean, whether to use C Locale or R session locale. Default TRUE.

## Value

Returns the input `x` in sorted order similar to
[`base::sort`](https://rdrr.io/r/base/sort.html) but usually faster. If
`c.locale=FALSE`, `psort` will return the same output as
[`base::sort`](https://rdrr.io/r/base/sort.html) with `method="quick"`,
i.e. using R session locale. If `c.locale=TRUE`, `psort` will return the
same output as [`base::sort`](https://rdrr.io/r/base/sort.html) with
`method="radix"`, i.e. using C locale. See example below.

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
