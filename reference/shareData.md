# Share Data between R Sessions

Share one R object between two R sessions on the same machine.

## Usage

``` r
shareData(data, map_name, verbose=FALSE)
getData(map_name, verbose=FALSE)
clearData(x, verbose=FALSE)
```

## Arguments

- data:

  An R object, for example a vector or a `data.frame`.

- map_name:

  A character string. The name of the shared memory object. The function
  adds a leading `/` if it is missing. Use a simple name. Do not use a
  path or a Windows `Global` or `Local` prefix. A name can describe only
  one object: calling `shareData()` again under a name that is in use
  overwrites that object (or fails if the new object is larger), and the
  older pointer's `clearData()` then leaves the newer object in place.

- x:

  An external pointer returned by `shareData`.

- verbose:

  A logical value. Use `TRUE` to print progress information, or `FALSE`
  to suppress it.

## Details

**This is a copy, not a live pointer.** `shareData()` serializes `data`
to a raw vector and copies it into shared memory. `getData()` copies the
raw vector back and unserializes it. The object is copied twice. Large
objects thus take time and memory. A plain pointer into the R heap of
another session cannot work, because R objects carry process-private
metadata (garbage collection, string cache, ALTREP, ...). Issue \#43
(<https://github.com/fastverse/kit/issues/43>) tracks the request for a
lightweight pointer-based alternative.

**Lifetime and platform differences.**

- POSIX (Linux, macOS, and other Unix-like systems):

  `getData()` unlinks the shared memory object before it calls
  [`unserialize()`](https://rdrr.io/r/base/serialize.html). A second
  call with the same `map_name` thus fails. The object stays alive while
  the pointer returned by `shareData()` exists. `clearData()` unmaps and
  unlinks the object it created, unless another session has since reused
  the name; then it only cleans up its own view. The same cleanup occurs
  when the pointer is garbage collected or the session exits. If the
  sharing session is killed, the object can stay in `/dev/shm` on Linux.
  It stays there until you remove it or the machine restarts.

- Windows:

  `getData()` closes its handles but does not delete the mapping. A
  second call thus succeeds while the sharing session is alive.
  `clearData()` removes the mapping when the last handle is closed. If
  the sharing session is killed, Windows closes its handles and the
  mapping disappears.

**Scope.** Sharing works only on one machine and for the same user. It
does not send data over a network. Use the same R version and the same
version of kit when possible. External pointers and connections do not
survive. Environments are copied, not shared.

**Memory limits.** The operating system limits shared memory. On Linux,
`/dev/shm` is often smaller than RAM, especially in containers. A large
object can thus fail even when the machine has enough memory.

The functions are currently tested on POSIX platforms (with `shm_open`)
and Windows (with named file mappings). Other platforms are untested.

## Value

`shareData` returns an external pointer. `getData` returns the R object
stored at `map_name`. `clearData` returns `TRUE` if it released the
pointer's mapping, or `FALSE` if the pointer was already cleared.

## Author

Morgan Jacob

## Examples

``` r
if (FALSE) { # \dontrun{
# In R session 1: share data in memory
x = shareData(mtcars, "share1")

# In R session 2: get data from session 1
getData("share1")

# In R session 1: clear data in memory
clearData(x)
} # }
```
