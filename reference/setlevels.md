# Set levels of a factor object

A function to set levels of a factor object.

## Usage

``` r
setlevels(x, old=levels(x), new, skip_absent=FALSE)
```

## Arguments

- x:

  A factor object.

- old:

  A character vector containing the factor levels to be changed. Default
  is levels of `x`.

- new:

  The new character vector containing the factor levels to be added.

- skip_absent:

  Skip items in `old` that are missing (i.e. absent) in \`names(x)\`.
  Default `FALSE` halts with error if any are missing.

## Value

Returns an invisible and modified factor object.

## Author

Morgan Jacob

## Examples

``` r
x = factor(c("A", "A", "B", "B", "B", "C")) # factor vector with levels A B C
setlevels(x, new = c("X", "Y", "Z"))        # set factor levels to: X Y Z
setlevels(x, old = "X", new = "A")          # set factor levels X to A
```
