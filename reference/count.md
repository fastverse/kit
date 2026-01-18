# count, countNA and countOccur

Simple functions to count the number of times an element occurs.

## Usage

``` r
count(x, value)
countNA(x)
countOccur(x)
```

## Arguments

- x:

  A vector or list for `countNA`. A vector for `count` and a vector or
  `data.frame` for `countOccur`.

- value:

  An element to look for. Must be non `NULL`, of length 1 and same type
  as `x`.

## Value

For a vector `countNA` will return the total number of `NA` value. For a
list, `countNA` will return a list with the number of `NA` in each item
of the list. This is a major difference with `sum(is.na(x))` which will
return the aggregated number of `NA`. Also, please note that every item
of a list can be of different type and `countNA` will take them into
account whether they are of type logical (`NA`), integer
(`NA_integer_`), double (`NA_real_`), complex (`NA_complex_`) or
character (`NA_character_`). As opposed to `countNA`, `count` does not
support list type and requires `x` and `value` to be of the same type.
Function `countOccur` takes vectors or data.frame as inputs and returns
a `data.frame` with the number of times each value in the vector occurs
or number of times each row in a `data.frame` occurs.

## See also

[`pcount`](https://fastverse.org/kit/reference/psum.md)

## Author

Morgan Jacob

## Examples

``` r
x = c(1, 3, NA, 5)
count(x, 3)
#> [1] 1

countNA(x)
#> [1] 1
countNA(as.list(x))
#> [[1]]
#> [1] 0
#> 
#> [[2]]
#> [1] 0
#> 
#> [[3]]
#> [1] 1
#> 
#> [[4]]
#> [1] 0
#> 

countOccur(x)
#>   Variable Count
#> 1        1     1
#> 2        3     1
#> 3       NA     1
#> 4        5     1

# Benchmarks countNA
# ------------------
# x = sample(c(TRUE,NA,FALSE),1e8,TRUE) # 382 Mb
# microbenchmark::microbenchmark(
#   countNA(x),
#   sum(is.na(x)),
#   times=5L
# )
# Unit: milliseconds
#          expr   min    lq   mean  median    uq   max neval
# countNA(x)     98.7  99.2  101.2   100.1 101.4 106.4     5
# sum(is.na(x)) 405.4 441.3  478.9   461.1 523.9 562.6     5
#
# Benchmarks countOccur
# ---------------------
# x = rnorm(1e6)
# y = data.table::data.table(x)
# microbenchmark::microbenchmark(
#   kit= countOccur(x),
#   data.table = y[, .N, keyby = x],
#   table(x),
#   times = 10L
# )
# Unit: milliseconds
# expr        min         lq     mean    median      uq     max neval
# kit          62.26   63.88    89.29     75.49   95.17  162.40    10
# data.table  189.17  194.08   235.30    227.43  263.74  337.74    10 # setDTthreads(1L)
# data.table  140.15  143.91   190.04    182.85  234.48  261.43    10 # setDTthreads(2L)
# table(x)   3560.77 3705.06  3843.47   3807.12 4048.40 4104.11    10
```
