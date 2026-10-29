# Top N values index

`topn` is used to get the indices of the few values of an input. This is
an extension of
[`which.max`](https://rdrr.io/r/base/which.min.html)/[`which.min`](https://rdrr.io/r/base/which.min.html)
which provide *only* the first such index.

The output is the same as `order(vec)[1:n]`, but internally optimized
not to sort the irrelevant elements of the input (and therefore much
faster, for small `n` relative to input size).

## Usage

``` r
topn(vec, n=6L, decreasing=TRUE, hasna=TRUE, index=TRUE)
```

## Arguments

- vec:

  A numeric vector of type numeric or integer. Other types are not
  supported yet.

- n:

  A positive integer value greater or equal to 1.

- decreasing:

  A logical value (default `TRUE`) to indicate whether to sort `vec` in
  decreasing or increasing order. Equivalent to argument `decreasing` in
  function [`base::order`](https://rdrr.io/r/base/order.html). Please
  note that unlike `topn` default value in
  [`base::order`](https://rdrr.io/r/base/order.html) is `FALSE`.

- hasna:

  A logical value (default `TRUE`) to indicate whether `vec` contains
  `NA` values.

- index:

  A logical value (default `TRUE`) to indicate whether indexes or values
  of `vec`.

## Value

`integer` vector of indices of the most extreme (according to
`decreasing`) `n` values in vector `vec`. Please note that for large
value of `n`, i.e. 1500 or 2000 (depending on the value of `hasna`),
`topn` will default to base R function `order`. It also does so when the
linear scan's rescan work becomes adversarial, e.g. for input already
ordered opposite to `decreasing`, so the worst case is bounded. The
fallback allocates a full-length `order` index (4 bytes per element:
about 80 MB at 2e7 elements and 8 GB near `.Machine$integer.max`), so
peak memory can be much higher than the linear path's memory
proportional to `n`. Classed numeric vectors have `xtfrm` applied once
and then use the same fast path, so their result matches `order`/`xtfrm`
without sorting the whole vector for small `n`; for inputs longer than
`.Machine$integer.max`, class attributes are ignored and the stored
values are compared.

For inputs longer than `.Machine$integer.max` the result is a `double`
vector of indices, as an integer cannot address the whole input,
computed with memory proportional to `n`: the selection heap needs about
24 bytes per entry (double value, long index, missing flag) and the
returned indices 8 bytes each. Note that for `n` larger than roughly
three quarters of the input length this can exceed the memory of a
full-length `order` index, and `order` (for `decreasing=FALSE`)
processes such inputs faster; the saving is the largest for small `n`.
The output matches
`order(vec, decreasing=decreasing, method="radix", na.last=TRUE)[1:n]`:
ties keep input order and missing values are placed last in input order;
this holds whatever the value of `hasna`, and
[`order`](https://rdrr.io/r/base/order.html) itself errors with
`decreasing=TRUE` for such inputs while `topn` does not.

## Author

Morgan Jacob

## Examples

``` r
x = rnorm(1e4)

# Example 1: index of top 6 negative values
topn(x, 6L, decreasing=FALSE)
#> [1] 3859 9372 5250 9081 4216 7502
order(x)[1:6]
#> [1] 3859 9372 5250 9081 4216 7502

# Example 2: index of top 6 positive values
topn(x, 6L, decreasing = TRUE)
#> [1] 9437 3328 4002 8436 6252 1202
order(x, decreasing=TRUE)[1:6]
#> [1] 9437 3328 4002 8436 6252 1202

# Example 3: top 6 negative values
topn(x, 6L, decreasing=FALSE, index=FALSE)
#> [1] -3.304182 -3.165100 -3.153828 -3.112280 -3.004550 -2.998378
sort(x)[1:6]
#> [1] -3.304182 -3.165100 -3.153828 -3.112280 -3.004550 -2.998378

# Benchmarks
# ----------
# x = rnorm(1e7) # 76Mb
# microbenchmark::microbenchmark(
#   topn=kit::topn(x, 6L),
#   order=order(x, decreasing=TRUE)[1:6],
#   times=10L
# )
# Unit: milliseconds
#  expr min   lq  mean median   uq  max neval
# topn   11   11    13     11   12   18    10
# order 563  565   587    566  602  661    10
#
# microbenchmark::microbenchmark(
#  topn=kit::topn(x, 6L, decreasing=FALSE, index=FALSE),
#  sort=sort(x, partial=1:6)[1:6],
#  times=10L
# )
# Unit: milliseconds
# expr min  lq  mean median   uq  max neval
# topn  11  11    11     11   12   12    10
# sort 167 175   197    178  205  303    10
```
