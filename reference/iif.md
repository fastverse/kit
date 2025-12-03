# Fast if else

`iif` is a faster and more robust replacement of
[`ifelse`](https://rdrr.io/r/base/ifelse.html). It is comparable to
`dplyr::if_else`, `hutils::if_else` and `data.table::fifelse`. It
returns a value with the same length as `test` filled with corresponding
values from `yes`, `no` or eventually `na`, depending on `test`. It does
not support S4 classes.

## Usage

``` r
iif(test, yes, no, na=NULL, tprom=FALSE, nThread=getOption("kit.nThread"))
```

## Arguments

- test:

  A logical vector.

- yes, no:

  Values to return depending on `TRUE`/`FALSE` element of `test`. They
  must be the same type and be either length `1` or the same length of
  `test`.

- na:

  Value to return if an element of `test` is missing. It must be the
  same type as `yes`/`no` and be either length `1` or the same length of
  `test`. Please note that `NA` is treated as logical value of length 1
  as per the R documentation. `NA_integer_`, `NA_real_`, `NA_complex_`
  and `NA_character_` are equivalent to `NA` but for integer, double,
  complex and character. Default value for argument `na` is `NULL` and
  will automatically default to the equivalent NA type of argument
  `yes`.

- tprom:

  Argument to indicate whether type promotion of `yes` and `no` is
  allowed or not. Either `FALSE` or `TRUE`, default is `FALSE` to not
  allow type promotion.

- nThread:

  A integer for the number of threads to use with *openmp*. Default
  value is `getOption("kit.nThread")`.

## Details

In contrast to [`ifelse`](https://rdrr.io/r/base/ifelse.html) attributes
are copied from `yes` to the output. This is useful when returning
`Date`, `factor` or other classes. Like `dplyr::if_else` and
`hutils::if_else`, the `na` argument is by default set to `NULL`. This
argument is set to `NA` in data.table::fifelse. Similarly to
`dplyr::if_else` and when `tprom=FALSE`, `iif` requires same type for
arguments `yes` and `no`. This is not strictly the case for
`data.table::fifelse` which will coerce integer to double. When
`tprom=TRUE`, `iif` behavior is similar to
[`base::ifelse`](https://rdrr.io/r/base/ifelse.html) in the sense that
it will promote or coerce `yes` and `no`to the "highest" used type.
Note, however, that unlike
[`base::ifelse`](https://rdrr.io/r/base/ifelse.html) attributes are
still conserved.

## Value

A vector of the same length as `test` and attributes as `yes`. Data
values are taken from the values of `yes` and `no`, eventually `na`.

## See also

[`nif`](https://fastverse.github.io/kit/reference/nif.md)
[`vswitch`](https://fastverse.github.io/kit/reference/vswitch.md)

## Author

Morgan Jacob

## Examples

``` r
x = c(1:4, 3:2, 1:4)
iif(x > 2L, x, x - 1L)
#>  [1] 0 1 3 4 3 1 0 1 3 4

# unlike ifelse, iif preserves attributes, taken from the 'yes' argument
dates = as.Date(c("2011-01-01","2011-01-02","2011-01-03","2011-01-04","2011-01-05"))
ifelse(dates == "2011-01-01", dates - 1, dates)
#> [1] 14974 14976 14977 14978 14979
iif(dates == "2011-01-01", dates - 1, dates)
#> [1] "2010-12-31" "2011-01-02" "2011-01-03" "2011-01-04" "2011-01-05"
yes = factor(c("a","b","c"))
no = yes[1L]
ifelse(c(TRUE,FALSE,TRUE), yes, no)
#> [1] 1 1 3
iif(c(TRUE,FALSE,TRUE), yes, no)
#> [1] a a c
#> Levels: a b c

# Example of using the 'na' argument
iif(test = c(-5L:5L < 0L, NA), yes = 1L, no = 0L, na = 2L)
#>  [1] 1 1 1 1 1 0 0 0 0 0 0 2

# Example of using the 'tprom' argument
iif(test = c(-5L:5L < 0L, NA), yes = 1L, no = "0", na = 2L, tprom = TRUE)
#>  [1] "1" "1" "1" "1" "1" "0" "0" "0" "0" "0" "0" "2"
```
