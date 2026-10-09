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
  data. Needs to start with a "/" on POSIX platforms, this is done
  automatically if missing.

- x:

  An external pointer like the one returned by function `shareData`.

- verbose:

  A logical value `TRUE` or `FALSE` to provide or not information to the
  user.

## Details

`shareData` is a deep copy, not a pointer: the object is serialized into
a raw vector in the sharing session and that raw vector is copied into a
shared memory object. `getData` copies the raw vector back into the
reading session and unserializes it. The full object is thus serialized
twice and copied once, which for large objects dominates the cost of the
transfer.

The shared object can be read once: `getData` unlinks the memory map as
soon as it has read the data, so calling `getData` again with the same
`map_name` fails.

The data lives as long as the external pointer returned by `shareData`
exists in the sharing session: `clearData` unmaps and unlinks the shared
memory object and returns `TRUE`, or `FALSE` if it was already cleared.
The same cleanup happens automatically when that pointer is garbage
collected or the session exits. If the sharing session is killed before
any of that, the shared memory object may persist (`/dev/shm` on POSIX)
until removed.

Functions are currently only tested on POSIX platforms (using
`shm_open`) and Windows (using named file mappings), other platforms are
untested.

## Value

`shareData` returns a external pointer. `getData` returns an R object
stored in the memory location `map_name`. `clearData` returns `TRUE` or
`FALSE` depending on whether the data have been cleared in memory.

## Note

Sharing objects is not zero-copy: a plain pointer into the R heap of
another session cannot work, because R objects carry process-private
metadata (garbage collection, string cache, ALTREP, ...). Issue \#43
(<https://github.com/fastverse/kit/issues/43>) tracks the request for a
lightweight pointer-based alternative.

## Author

Morgan Jacob

## Examples

``` r
if (FALSE) { # \dontrun{
# In R session 1: share data in memory
x = shareData(mtcars,"share1")

# In R session 2: get data from session 1 (readable only once!)
getData("share1")

# In R session 1: clear data in memory
clearData(x)
} # }
```
