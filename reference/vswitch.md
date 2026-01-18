# Vectorised switch

`vswitch`/ `nswitch` is a vectorised version of `base` function
`switch`. This function can also be seen as a particular case of
function `nif`, as shown in examples below, and should also be faster.

## Usage

``` r
vswitch(x, values, outputs, default=NULL,
        nThread=getOption("kit.nThread"),
        checkEnc=TRUE)
nswitch(x, ..., default=NULL,
        nThread=getOption("kit.nThread"),
        checkEnc=TRUE)
```

## Arguments

- x:

  A vector or list.

- values:

  A vector or list with values from `x` to match. Note that `x` and
  `values` must have the same class and attributes.

- outputs:

  A list or vector with the outputs to return for every matching values.
  Each item of the list must be of length 1 or length of `x`. Note that
  if all list items are of length 1 then it might be simpler to use a
  vector.

- ...:

  A sequence of values and outputs in the following order
  `value1, output1, value2, output2, ..., valueN, outputN`. Values
  `value1, value2, ..., valueN` must all have length1, same type and
  attributes. Each `output` may either share length with `x` or be
  length 1. Please see Examples section for further details.

- default:

  Values to return is no match. Must be a vector or list of length 1 or
  same length as `x`. Also, `default` must have the same type, class and
  attributes as items from `outputs`.

- nThread:

  A integer for the number of threads to use with *openmp*. Default
  value is `getOption("kit.nThread")`.

- checkEnc:

  A logical value whether or not to check if `x` and `values` have
  comparable and consistent encoding. Default is `TRUE`.

## Value

A vector or list of the same length as `x` with values from `outputs`
items and from `default` if missing.

## See also

[`iif`](https://fastverse.org/kit/reference/iif.md)
[`nif`](https://fastverse.org/kit/reference/nif.md)

## Author

Morgan Jacob

## Examples

``` r
x = sample(c(10L, 20L, 30L, 40L, 50L, 60L), 3e2, replace=TRUE)

# The below example of 'vswitch' is
a1 = vswitch(
  x = x,
  values = c(10L,20L,30L,40L,50L),
  outputs = c(11L,21L,31L,41L,51L),
  default = NA_integer_
)

# equivalent to the following 'nif' example.
# However for large vectors 'vswitch' should be faster.
b1 = nif(
  x==10L, 11L,
  x==20L, 21L,
  x==30L, 31L,
  x==40L, 41L,
  x==50L, 51L,
  default = NA_integer_
)
identical(a1, b1)
#> [1] TRUE

# nswitch can also be used as follows:
c1 = nswitch(x,
  10L, 11L,
  20L, 21L,
  30L, 31L,
  40L, 41L,
  50L, 51L,
  default = NA_integer_
)
identical(a1, c1)
#> [1] TRUE

# Example with list in 'outputs' argument
y = c(1, 0, NA_real_)
a2 = vswitch(
  x = y,
  values = c(1, 0),
  outputs = list(c(2, 3, 4), c(5, 6, 7)),
  default = 8
)

b2 = nif(
  y==1, c(2, 3, 4),
  y==0, c(5, 6, 7),
  default = 8
)

identical(a2, b2)
#> [1] TRUE

c2 = nswitch(y,
  1, c(2, 3, 4),
  0, c(5, 6, 7),
  default = 8
)

identical(a2, c2)
#> [1] TRUE

# Benchmarks
# ----------
# x = sample(1:100, 3e8, TRUE) # 1.1Gb
# microbenchmark::microbenchmark(
# nif=kit::nif(
#   x==10L,  0L,
#   x==20L, 10L,
#   x==30L, 20L,
#   default= 30L
#  ),
# vswitch=kit::vswitch(
#   x, c( 10L, 20L, 30L), list(0L, 10L, 20L), 30L
# ),
# times=10L
# )
# Unit: seconds
#    expr   min    lq  mean median   uq  max neval
# nif      4.27  4.37  4.43   4.42 4.52 4.53    10
# vswitch  1.08  1.09  1.20   1.10 1.43 1.44    10 # 1 thread
# vswitch  0.46  0.57  0.57   0.58 0.58 0.60    10 # 2 threads
```
