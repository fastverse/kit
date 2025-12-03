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
`topn` will default to base R function `order`.

## Author

Morgan Jacob

## Examples

``` r
x = rnorm(1e4)

# Example 1: index of top 6 negative values
topn(x, 6L, decreasing=FALSE)
#> [1] 3848 9361 5239 9070 4205 7491
order(x)[1:6]
#> [1] 3848 9361 5239 9070 4205 7491

# Example 2: index of top 6 positive values
topn(x, 6L, decreasing = TRUE)
#> [1] 9426 3317 3991 8425 6241 1191
order(x, decreasing=TRUE)[1:6]
#> [1] 9426 3317 3991 8425 6241 1191

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
