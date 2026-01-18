# Parallel (Statistical) Functions

Vector-valued (statistical) functions operating in parallel over vectors
passed as arguments, or a single list of vectors (such as a data frame).
Similar to [`pmin`](https://rdrr.io/r/base/Extremes.html) and
[`pmax`](https://rdrr.io/r/base/Extremes.html), except that these
functions do not recycle vectors.

## Usage

``` r
psum(..., na.rm = FALSE)
pprod(..., na.rm = FALSE)
fpmin(..., na.rm = FALSE)
fpmax(..., na.rm = FALSE)
prange(..., na.rm = FALSE)
pmean(..., na.rm = FALSE)
pfirst(...)  # (na.rm = TRUE)
plast(...)   # (na.rm = TRUE)
pall(..., na.rm = FALSE)
pallNA(...)
pallv(..., value)
pany(..., na.rm = FALSE)
panyNA(...)
panyv(..., value)
pcount(..., value)
pcountNA(...)
```

## Arguments

- ...:

  suitable (atomic) vectors of the same length, or a single list of
  vectors (such as a `data.frame`). See Details on the allowed data
  types for each function, and Examples.

- na.rm:

  A logical indicating whether missing values should be removed. Default
  value is `FALSE`, except for `pfirst` and `plast`.

- value:

  A non `NULL` value of length 1.

## Details

Functions `psum`, `pprod` work for integer, logical, double and complex
types. `pmean`, `fpmin`, `fpmax`, and `prange` only support integer,
logical and double types. All these functions will error if used with
factors.

`pfirst`/`plast` select the first/last non-missing value (or non-empty
or `NULL` value for list-vectors). They accept all vector types with
defined missing values + lists, but can only jointly handle integer and
double types (not numeric and complex or character and factor). If
factors are passed, they all need to have identical levels.

`pany` and `pall` are derived from base functions `all` and `any` and
only allow logical inputs.

`pcount` counts the occurrence of `value`, and expects arguments of the
same data type (except for `value = NA`). `pcountNA` is equivalent to
`pcount` with `value = NA`, and they both allow `NA` counting in
mixed-type data. `pcountNA` additionally supports list vectors and
counts empty or `NULL` elements as `NA`.

Functions `panyv/pallv` are wrappers around `pcount`, and
`panyNA/pallNA` are wrappers around `pcountNA`. They return a logical
vector instead of the integer count.

None of these functions recycle vectors i.e. all input vectors need to
have the same length. All functions support long vectors with up to
`2^64-1` elements.

## Value

`psum/pprod/pmean` return the sum, product or mean of all arguments. The
value returned will be of the highest argument type (integer \< double
\< complex). `pprod` only returns double or complex. `fpmin/fpmax`
return the element-wise minimum/maximum of all arguments, with the same
type as the inputs if all inputs have the same type, otherwise the
highest type (logical \< integer \< double). `prange` returns the
element-wise range (max - min) as a double vector. `pall[v/NA]` and
`pany[v/NA]` return a logical vector. `pcount[NA]` returns an integer
vector. `pfirst/plast` return a vector of the same type as the inputs.

## See also

Package 'collapse' provides column-wise and scalar-valued analogues to
many of these functions.

## Author

Morgan Jacob and Sebastian Krantz

## Examples

``` r
x = c(1, 3, NA, 5)
y = c(2, NA, 4, 1)
z = c(3, 4, 4, 1)

# Example 1: psum
psum(x, y, z, na.rm = FALSE)
#> [1]  6 NA NA  7
psum(x, y, z, na.rm = TRUE)
#> [1] 6 7 8 7

# Example 2: pprod
pprod(x, y, z, na.rm = FALSE)
#> [1]  6 NA NA  5
pprod(x, y, z, na.rm = TRUE)
#> [1]  6 12 16  5

# Example 3: pmean
pmean(x, y, z, na.rm = FALSE)
#> [1] 2.000000       NA       NA 2.333333
pmean(x, y, z, na.rm = TRUE)
#> [1] 2.000000 3.500000 4.000000 2.333333

# Example 3b: fpmin, fpmax, prange
fpmin(x, y, z, na.rm = FALSE)
#> [1]   1 NaN NaN   1
fpmin(x, y, z, na.rm = TRUE)
#> [1] 1 3 4 1
fpmax(x, y, z, na.rm = FALSE)
#> [1]   3 NaN NaN   5
fpmax(x, y, z, na.rm = TRUE)
#> [1] 3 4 4 5
prange(x, y, z, na.rm = FALSE)
#> [1]   2 NaN NaN   4
prange(x, y, z, na.rm = TRUE)
#> [1] 2 1 0 4

# Example 4: pfirst and plast
pfirst(x, y, z)
#> [1] 1 3 4 5
plast(x, y, z)
#> [1] 3 4 4 1

# Adjust x, y, and z to use in pall and pany
x = c(TRUE, FALSE, NA, FALSE)
y = c(TRUE, NA, TRUE, TRUE)
z = c(TRUE, TRUE, FALSE, NA)

# Example 5: pall
pall(x, y, z, na.rm = FALSE)
#> [1]  TRUE FALSE FALSE FALSE
pall(x, y, z, na.rm = TRUE)
#> [1]  TRUE FALSE FALSE FALSE

# Example 6: pany
pany(x, y, z, na.rm = FALSE)
#> [1] TRUE TRUE TRUE TRUE
pany(x, y, z, na.rm = TRUE)
#> [1] TRUE TRUE TRUE TRUE

# Example 7: pcount
pcount(x, y, z, value = TRUE)
#> [1] 3 1 1 1
pcountNA(x, y, z)
#> [1] 0 1 1 1

# Example 8: list/data.frame as an input
pprod(iris[,1:2])
#>   [1] 17.85 14.70 15.04 14.26 18.00 21.06 15.64 17.00 12.76 15.19 19.98 16.32
#>  [13] 14.40 12.90 23.20 25.08 21.06 17.85 21.66 19.38 18.36 18.87 16.56 16.83
#>  [25] 16.32 15.00 17.00 18.20 17.68 15.04 14.88 18.36 21.32 23.10 15.19 16.00
#>  [37] 19.25 17.64 13.20 17.34 17.50 10.35 14.08 17.50 19.38 14.40 19.38 14.72
#>  [49] 19.61 16.50 22.40 20.48 21.39 12.65 18.20 15.96 20.79 11.76 19.14 14.04
#>  [61] 10.00 17.70 13.20 17.69 16.24 20.77 16.80 15.66 13.64 14.00 18.88 17.08
#>  [73] 15.75 17.08 18.56 19.80 19.04 20.10 17.40 14.82 13.20 13.20 15.66 16.20
#>  [85] 16.20 20.40 20.77 14.49 16.80 13.75 14.30 18.30 15.08 11.50 15.12 17.10
#>  [97] 16.53 17.98 12.75 15.96 20.79 15.66 21.30 18.27 19.50 22.80 12.25 21.17
#> [109] 16.75 25.92 20.80 17.28 20.40 14.25 16.24 20.48 19.50 29.26 20.02 13.20
#> [121] 22.08 15.68 21.56 17.01 22.11 23.04 17.36 18.30 17.92 21.60 20.72 30.02
#> [133] 17.92 17.64 15.86 23.10 21.42 19.84 18.00 21.39 20.77 21.39 15.66 21.76
#> [145] 22.11 20.10 15.75 19.50 21.08 17.70
psum(iris[,1:2])
#>   [1]  8.6  7.9  7.9  7.7  8.6  9.3  8.0  8.4  7.3  8.0  9.1  8.2  7.8  7.3  9.8
#>  [16] 10.1  9.3  8.6  9.5  8.9  8.8  8.8  8.2  8.4  8.2  8.0  8.4  8.7  8.6  7.9
#>  [31]  7.9  8.8  9.3  9.7  8.0  8.2  9.0  8.5  7.4  8.5  8.5  6.8  7.6  8.5  8.9
#>  [46]  7.8  8.9  7.8  9.0  8.3 10.2  9.6 10.0  7.8  9.3  8.5  9.6  7.3  9.5  7.9
#>  [61]  7.0  8.9  8.2  9.0  8.5  9.8  8.6  8.5  8.4  8.1  9.1  8.9  8.8  8.9  9.3
#>  [76]  9.6  9.6  9.7  8.9  8.3  7.9  7.9  8.5  8.7  8.4  9.4  9.8  8.6  8.6  8.0
#>  [91]  8.1  9.1  8.4  7.3  8.3  8.7  8.6  9.1  7.6  8.5  9.6  8.5 10.1  9.2  9.5
#> [106] 10.6  7.4 10.2  9.2 10.8  9.7  9.1  9.8  8.2  8.6  9.6  9.5 11.5 10.3  8.2
#> [121] 10.1  8.4 10.5  9.0 10.0 10.4  9.0  9.1  9.2 10.2 10.2 11.7  9.2  9.1  8.7
#> [136] 10.7  9.7  9.5  9.0 10.0  9.8 10.0  8.5 10.0 10.0  9.7  8.8  9.5  9.6  8.9
pmean(iris[,1:2])
#>   [1] 4.30 3.95 3.95 3.85 4.30 4.65 4.00 4.20 3.65 4.00 4.55 4.10 3.90 3.65 4.90
#>  [16] 5.05 4.65 4.30 4.75 4.45 4.40 4.40 4.10 4.20 4.10 4.00 4.20 4.35 4.30 3.95
#>  [31] 3.95 4.40 4.65 4.85 4.00 4.10 4.50 4.25 3.70 4.25 4.25 3.40 3.80 4.25 4.45
#>  [46] 3.90 4.45 3.90 4.50 4.15 5.10 4.80 5.00 3.90 4.65 4.25 4.80 3.65 4.75 3.95
#>  [61] 3.50 4.45 4.10 4.50 4.25 4.90 4.30 4.25 4.20 4.05 4.55 4.45 4.40 4.45 4.65
#>  [76] 4.80 4.80 4.85 4.45 4.15 3.95 3.95 4.25 4.35 4.20 4.70 4.90 4.30 4.30 4.00
#>  [91] 4.05 4.55 4.20 3.65 4.15 4.35 4.30 4.55 3.80 4.25 4.80 4.25 5.05 4.60 4.75
#> [106] 5.30 3.70 5.10 4.60 5.40 4.85 4.55 4.90 4.10 4.30 4.80 4.75 5.75 5.15 4.10
#> [121] 5.05 4.20 5.25 4.50 5.00 5.20 4.50 4.55 4.60 5.10 5.10 5.85 4.60 4.55 4.35
#> [136] 5.35 4.85 4.75 4.50 5.00 4.90 5.00 4.25 5.00 5.00 4.85 4.40 4.75 4.80 4.45
fpmin(iris[,1:2])
#>   [1] 3.5 3.0 3.2 3.1 3.6 3.9 3.4 3.4 2.9 3.1 3.7 3.4 3.0 3.0 4.0 4.4 3.9 3.5
#>  [19] 3.8 3.8 3.4 3.7 3.6 3.3 3.4 3.0 3.4 3.5 3.4 3.2 3.1 3.4 4.1 4.2 3.1 3.2
#>  [37] 3.5 3.6 3.0 3.4 3.5 2.3 3.2 3.5 3.8 3.0 3.8 3.2 3.7 3.3 3.2 3.2 3.1 2.3
#>  [55] 2.8 2.8 3.3 2.4 2.9 2.7 2.0 3.0 2.2 2.9 2.9 3.1 3.0 2.7 2.2 2.5 3.2 2.8
#>  [73] 2.5 2.8 2.9 3.0 2.8 3.0 2.9 2.6 2.4 2.4 2.7 2.7 3.0 3.4 3.1 2.3 3.0 2.5
#>  [91] 2.6 3.0 2.6 2.3 2.7 3.0 2.9 2.9 2.5 2.8 3.3 2.7 3.0 2.9 3.0 3.0 2.5 2.9
#> [109] 2.5 3.6 3.2 2.7 3.0 2.5 2.8 3.2 3.0 3.8 2.6 2.2 3.2 2.8 2.8 2.7 3.3 3.2
#> [127] 2.8 3.0 2.8 3.0 2.8 3.8 2.8 2.8 2.6 3.0 3.4 3.1 3.0 3.1 3.1 3.1 2.7 3.2
#> [145] 3.3 3.0 2.5 3.0 3.4 3.0
fpmax(iris[,1:2])
#>   [1] 5.1 4.9 4.7 4.6 5.0 5.4 4.6 5.0 4.4 4.9 5.4 4.8 4.8 4.3 5.8 5.7 5.4 5.1
#>  [19] 5.7 5.1 5.4 5.1 4.6 5.1 4.8 5.0 5.0 5.2 5.2 4.7 4.8 5.4 5.2 5.5 4.9 5.0
#>  [37] 5.5 4.9 4.4 5.1 5.0 4.5 4.4 5.0 5.1 4.8 5.1 4.6 5.3 5.0 7.0 6.4 6.9 5.5
#>  [55] 6.5 5.7 6.3 4.9 6.6 5.2 5.0 5.9 6.0 6.1 5.6 6.7 5.6 5.8 6.2 5.6 5.9 6.1
#>  [73] 6.3 6.1 6.4 6.6 6.8 6.7 6.0 5.7 5.5 5.5 5.8 6.0 5.4 6.0 6.7 6.3 5.6 5.5
#>  [91] 5.5 6.1 5.8 5.0 5.6 5.7 5.7 6.2 5.1 5.7 6.3 5.8 7.1 6.3 6.5 7.6 4.9 7.3
#> [109] 6.7 7.2 6.5 6.4 6.8 5.7 5.8 6.4 6.5 7.7 7.7 6.0 6.9 5.6 7.7 6.3 6.7 7.2
#> [127] 6.2 6.1 6.4 7.2 7.4 7.9 6.4 6.3 6.1 7.7 6.3 6.4 6.0 6.9 6.7 6.9 5.8 6.8
#> [145] 6.7 6.7 6.3 6.5 6.2 5.9
prange(iris[,1:2])
#>   [1] 1.6 1.9 1.5 1.5 1.4 1.5 1.2 1.6 1.5 1.8 1.7 1.4 1.8 1.3 1.8 1.3 1.5 1.6
#>  [19] 1.9 1.3 2.0 1.4 1.0 1.8 1.4 2.0 1.6 1.7 1.8 1.5 1.7 2.0 1.1 1.3 1.8 1.8
#>  [37] 2.0 1.3 1.4 1.7 1.5 2.2 1.2 1.5 1.3 1.8 1.3 1.4 1.6 1.7 3.8 3.2 3.8 3.2
#>  [55] 3.7 2.9 3.0 2.5 3.7 2.5 3.0 2.9 3.8 3.2 2.7 3.6 2.6 3.1 4.0 3.1 2.7 3.3
#>  [73] 3.8 3.3 3.5 3.6 4.0 3.7 3.1 3.1 3.1 3.1 3.1 3.3 2.4 2.6 3.6 4.0 2.6 3.0
#>  [91] 2.9 3.1 3.2 2.7 2.9 2.7 2.8 3.3 2.6 2.9 3.0 3.1 4.1 3.4 3.5 4.6 2.4 4.4
#> [109] 4.2 3.6 3.3 3.7 3.8 3.2 3.0 3.2 3.5 3.9 5.1 3.8 3.7 2.8 4.9 3.6 3.4 4.0
#> [127] 3.4 3.1 3.6 4.2 4.6 4.1 3.6 3.5 3.5 4.7 2.9 3.3 3.0 3.8 3.6 3.8 3.1 3.6
#> [145] 3.4 3.7 3.8 3.5 2.8 2.9

# Benchmarks
# ----------
# n = 1e8L
# x = rnorm(n) # 763 Mb
# y = rnorm(n)
# z = rnorm(n)
#
# microbenchmark::microbenchmark(
#   kit=psum(x, y, z, na.rm = TRUE),
#   base=rowSums(do.call(cbind,list(x, y, z)), na.rm=TRUE),
#   times = 5L, unit = "s"
# )
# Unit: Second
# expr  min   lq mean median   uq  max neval
# kit  0.52 0.52 0.65   0.55 0.83 0.84     5
# base 2.16 2.27 2.34   2.35 2.43 2.49     5
#
# x = sample(c(TRUE, FALSE, NA), n, TRUE) # 382 Mb
# y = sample(c(TRUE, FALSE, NA), n, TRUE)
# z = sample(c(TRUE, FALSE, NA), n, TRUE)
#
# microbenchmark::microbenchmark(
#   kit=pany(x, y, z, na.rm = TRUE),
#   base=sapply(1:n, function(i) any(x[i],y[i],z[i],na.rm=TRUE)),
#   times = 5L
# )
# Unit: Second
# expr    min     lq   mean   median     uq    max neval
# kit    1.07   1.09   1.15     1.10   1.23   1.23     5
# base 111.31 112.02 112.78   112.97 113.55 114.03     5
```
