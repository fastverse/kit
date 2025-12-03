# Nested if else

`nif` is a fast implementation of SQL `CASE WHEN` statement for R.
Conceptually, `nif` is a nested version of
[`iif`](https://fastverse.github.io/kit/reference/iif.md) (with smarter
implementation than manual nesting). It is not the same but it is
comparable to `dplyr::case_when` and `data.table::fcase`.

## Usage

``` r
nif(..., default=NULL)
```

## Arguments

- ...:

  A sequence consisting of logical condition (`when`)-resulting value
  (`value`) *pairs* in the following order
  `when1, value1, when2, value2, ..., whenN, valueN`. Logical conditions
  `when1, when2, ..., whenN` must all have the same length, type and
  attributes. Each `value` may either share length with `when` or be
  length 1. Please see Examples section for further details.

- default:

  Default return value, `NULL` by default, for when all of the logical
  conditions `when1, when2, ..., whenN` are `FALSE` or missing for some
  entries. Argument `default` can be a vector either of length 1 or
  length of logical conditions `when1, when2, ..., whenN`. Note that
  argument 'default' must be named explicitly.

## Value

Vector with the same length as the logical conditions (`when`) in `...`,
filled with the corresponding values (`value`) from `...`, or eventually
`default`. Attributes of output values `value1, value2, ...valueN` in
`...` are preserved.

## Details

Unlike `data.table::fcase`, the `default` argument is set to `NULL`. In
addition, `nif` can be called by other packages at C level. Note that at
C level, the function has an additional argument `SEXP md` which is
either `TRUE` for lazy evaluation or `FALSE` for non lazy evaluation.
This argument is not exposed to R users and is more for C users.

## See also

[`iif`](https://fastverse.github.io/kit/reference/iif.md)
[`vswitch`](https://fastverse.github.io/kit/reference/vswitch.md)

## Author

Morgan Jacob

## Examples

``` r
x = 1:10
nif(
  x < 5L, 1L,
  x > 5L, 3L
)
#>  [1]  1  1  1  1 NA  3  3  3  3  3

nif(
  x < 5L, 1L:10L,
  x > 5L, 3L:12L
)
#>  [1]  1  2  3  4 NA  8  9 10 11 12

# Lazy evaluation example
nif(
  x < 5L, 1L,
  x >= 5L, 3L,
  x == 5L, stop("provided value is an unexpected one!")
)
#>  [1] 1 1 1 1 3 3 3 3 3 3

# nif preserves attributes, example with dates
nif(
  x < 5L, as.Date("2019-10-11"),
  x > 5L, as.Date("2019-10-14")
)
#>  [1] "2019-10-11" "2019-10-11" "2019-10-11" "2019-10-11" NA          
#>  [6] "2019-10-14" "2019-10-14" "2019-10-14" "2019-10-14" "2019-10-14"

# nif example with factor; note the matching levels
nif(
  x < 5L, factor("a", levels=letters[1:3]),
  x > 5L, factor("b", levels=letters[1:3])
)
#>  [1] a    a    a    a    <NA> b    b    b    b    b   
#> Levels: a b c

# Example of using the 'default' argument
nif(
  x < 5L, 1L,
  x > 5L, 3L,
  default = 5L
)
#>  [1] 1 1 1 1 5 3 3 3 3 3

nif(
  x < 5L, 1L,
  x > 5L, 3L,
  default = rep(5L, 10L)
)
#>  [1] 1 1 1 1 5 3 3 3 3 3
```
