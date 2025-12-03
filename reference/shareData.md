# Share Data between R Sessions

Experimental functions that enable the user to share a R object between
2 R sessions.

## Usage

``` r
shareData(data, map_name, verbose=FALSE)
getData(map_name, verbose=FALSE)
clearData(x, verbose=FALSE)
```

## Arguments

- data:

  A R object like a vector or a `data.frame`.

- map_name:

  A character. A name for the memory map location where to store the
  data.

- x:

  An external pointer like the one returned by function `shareData`.

- verbose:

  A logical value `TRUE` or `FALSE` to provide or not information to the
  user.

## Value

`shareData` returns a external pointer. `getData` returns an R object
stored in the memory location `map_name`. `clearData` returns `TRUE` or
`FALSE` depending on whether the data have been cleared in memory.

## Author

Morgan Jacob

## Examples

``` r
# In R session 1: share data in memory
# > x = shareData(mtcars,"share1")
#
# In R session 2: get data from session 1
# > getData("share1")
#
# In R session 1: clear data in memory
# > clearData(x)
```
