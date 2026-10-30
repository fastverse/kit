# Top N values index

`topn` returns the indices (or values) of the `n` largest or smallest
values in a vector. This extends
[`which.max`](https://rdrr.io/r/base/which.min.html) and
[`which.min`](https://rdrr.io/r/base/which.min.html), which return only
the first such index.

The resulting order is the same as
`order(vec, decreasing = decreasing, na.last = TRUE)[1:n]` when `hasna`
is `TRUE` or `vec` has no missing values, but `topn()` avoids sorting
the whole input. It is therefore much faster when `n` is small relative
to the input size.

## Usage

``` r
topn(vec, n=6L, decreasing=TRUE, hasna=TRUE, index=TRUE)
```

## Arguments

- vec:

  A numeric or integer vector. Other types are not supported yet.

- n:

  A positive integer greater than or equal to 1.

- decreasing:

  Logical. If `TRUE` (default), sort from largest to smallest. This is
  the same as `decreasing` in
  [`base::order`](https://rdrr.io/r/base/order.html), whose default is
  `FALSE`.

- hasna:

  Logical. Whether `vec` contains `NA` values. Default is `TRUE`.

- index:

  Logical. If `TRUE` (default), return indices. If `FALSE`, return
  values.

## Details

For small `n`, `topn()` uses memory proportional to `n`. It falls back
to [`base::order()`](https://rdrr.io/r/base/order.html) when `n > 1500`
and `hasna = TRUE`, or when `n > 2000` and `hasna = FALSE`. It also
falls back when the selection scan has to rescan too often, for example
when `vec` is already sorted in the opposite direction. The fallback
allocates a full-length index: about 4 bytes per element. This can use
much more memory than the fast path.

For inputs longer than `.Machine$integer.max`, `topn()` keeps a heap of
at most `n` entries instead, so memory stays proportional to `n` (about
24 bytes per entry plus 8 bytes per returned index). Class attributes
are ignored and the stored values are compared. Unlike
[`base::order()`](https://rdrr.io/r/base/order.html),
`decreasing = TRUE` works for these inputs. For `n` above roughly three
quarters of the input length this heap can use more memory than a
full-length [`order()`](https://rdrr.io/r/base/order.html) index, and
[`order()`](https://rdrr.io/r/base/order.html) is faster for
`decreasing = FALSE`.

For classed numeric vectors shorter than `.Machine$integer.max`,
[`xtfrm()`](https://rdrr.io/r/base/xtfrm.html) is applied once before
selection, so their order matches
[`order()`](https://rdrr.io/r/base/order.html)/[`xtfrm()`](https://rdrr.io/r/base/xtfrm.html).

## Value

If `index = TRUE`, an integer vector of the indices of the `n` largest
(or smallest) values in `vec`. If `index = FALSE`, the values
themselves. For inputs longer than `.Machine$integer.max`, the indices
are returned as a double vector.

Ties keep their input order. With `hasna = TRUE` (default), missing
values are placed last, in input order. `hasna = FALSE` asserts that
`vec` has no missing values: it is not checked, and for unclassed
vectors `NA` is then compared as an ordinary value. For inputs longer
than `.Machine$integer.max`, missing values are always placed last,
whatever the value of `hasna`. For inputs no longer than
`.Machine$integer.max`, the resulting order matches
`order(vec, decreasing = decreasing, method = "radix", na.last = TRUE)[1:n]`
when `vec` has no missing values or `hasna = TRUE`.

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
