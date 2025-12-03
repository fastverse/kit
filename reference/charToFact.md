# Convert Character Vector to Factor

Similar to [`base::as.factor`](https://rdrr.io/r/base/factor.html) but
much faster and only for converting character vector to factor.

## Usage

``` r
charToFact(x, decreasing=FALSE, addNA=TRUE,
           nThread=getOption("kit.nThread"))
```

## Arguments

- x:

  A vector of type character

- decreasing:

  A boolean. Whether to order levels in decreasing order or not. Default
  is `FALSE`.

- addNA:

  A boolean. Whether to include `NA` in levels of the output or not.
  Default is `TRUE`.

- nThread:

  Number of thread to use.

## Value

The character vector input as a factor. Please note that, unlike
`as.factor`, `NA` levels are preserved by default, however this can be
changed by setting argument `addNA` to `FALSE`.

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
