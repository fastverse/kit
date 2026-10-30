# Convert Character Vector to Factor

Similar to [`base::as.factor`](https://rdrr.io/r/base/factor.html), but
much faster and only for converting a character vector to a factor.

## Usage

``` r
charToFact(x, decreasing=FALSE, addNA=TRUE,
           nThread=getOption("kit.nThread"))
```

## Arguments

- x:

  A character vector.

- decreasing:

  Logical. If `TRUE`, order levels in decreasing order. Default is
  `FALSE`.

- addNA:

  Logical. If `TRUE` (default), include `NA` in the levels of the
  output.

- nThread:

  Integer. Number of threads to use. Default is
  `getOption("kit.nThread")`.

## Details

Levels are sorted in the session locale. For non-ASCII text, the level
order can therefore differ between operating systems and locales. If you
need a fixed order, set the levels yourself with
`factor(x, levels = ...)`.

With `addNA = TRUE` (default), `NA` is a level, unlike
[`as.factor()`](https://rdrr.io/r/base/factor.html), which drops `NA`
levels.

## Value

A factor with the same length as `x`.

## Examples

``` r
x = c("b","A","B","a","\xe4","a")
Encoding(x) = "latin1"
identical(charToFact(x), as.factor(x))
#> [1] TRUE
identical(charToFact(c("a","b",NA,"a")), addNA(as.factor(c("a","b",NA,"a"))))
#> [1] TRUE
identical(charToFact(c("a","b",NA,"a"), addNA=FALSE), as.factor(c("a","b",NA,"a")))
#> [1] TRUE

# Benchmarks
# ----------
# x = sample(letters,3e7,TRUE)
# microbenchmark::microbenchmark(
#   kit=kit::charToFact(x,nThread = 1L),
#   base=as.factor(x),
#   times = 5L
# )
# Unit: milliseconds
# expr  min   lq   mean  median   uq  max neval
# kit   188  190    196     194  200  208     5
# base 1402 1403   1455    1414 1420 1637     5
```
