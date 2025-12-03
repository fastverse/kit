# Fast duplicated and unique

Similar to base R functions `duplicated` and `unique`, `fduplicated` and
`funique` are slightly faster for vectors and much faster for
`data.frame`. Function `uniqLen` is equivalent to base R
`length(unique)` or `data.table::uniqueN`.

## Usage

``` r
fduplicated(x, fromLast = FALSE)
funique(x, fromLast = FALSE)
uniqLen(x)
```

## Arguments

- x:

  A vector, data.frame or matrix.

- fromLast:

  A logical value to indicate whether the search should start from the
  end or beginning. Default is `FALSE`.

## Value

Function `fduplicated` returns a logical vector and `funique` returns a
vector of the same type as `x` without the duplicated value. Function
`uniqLen` returns an integer.

## Author

Morgan Jacob

## Examples

``` r
# Example 1: fduplicated
fduplicated(iris$Species)
#>   [1] FALSE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [13]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [25]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [37]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [49]  TRUE  TRUE FALSE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [61]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [73]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [85]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#>  [97]  TRUE  TRUE  TRUE  TRUE FALSE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#> [109]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#> [121]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#> [133]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE
#> [145]  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE

# Example 2: funique
funique(iris$Species)
#> [1] setosa     versicolor virginica 
#> Levels: setosa versicolor virginica

# Example 3: uniqLen
uniqLen(iris$Species)
#> [1] 3

# Benchmarks
# ----------
# x = sample(c(1:10,NA_integer_),1e8,TRUE) # 382 Mb
# microbenchmark::microbenchmark(
#   duplicated(x),
#   fduplicated(x),
#   times = 5L
# )
# Unit: seconds
#           expr  min   lq  mean  median   uq   max neval
# duplicated(x)  2.21 2.21  2.48    2.21 2.22  3.55     5
# fduplicated(x) 0.38 0.39  0.45    0.48 0.49  0.50     5
#
# vs data.table
# -------------
# df = iris[,5:1]
# for (i in 1:16) df = rbind(df, df)  # 338 Mb
# dt = data.table::as.data.table(df)
# microbenchmark::microbenchmark(
#   kit = funique(df),
#   data.table = unique(dt),
#   times = 5L
# )
# Unit: seconds
#       expr  min   lq  mean  median    uq  max neval
# kit        1.22 1.27  1.33    1.27  1.36 1.55     5
# data.table 6.20 6.24  6.43    6.33  6.46 6.93     5 # (setDTthreads(1L))
# data.table 4.20 4.25  4.47    4.26  4.32 5.33     5 # (setDTthreads(2L))
#
# microbenchmark::microbenchmark(
#   kit=uniqLen(x),
#   data.table=uniqueN(x),
#   times = 5L, unit = "s"
# )
# Unit: seconds
#       expr  min    lq  mean  median   uq  max neval
# kit        0.17  0.17  0.17   0.17  0.17 0.17     5
# data.table 1.66  1.68  1.70   1.71  1.71 1.72     5 # (setDTthreads(1L))
# data.table 1.13  1.15  1.16   1.16  1.18 1.18     5 # (setDTthreads(2L))
```
