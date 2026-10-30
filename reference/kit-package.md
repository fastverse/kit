# kit: Fast Data Manipulation Functions in C

kit provides fast, vectorised functions for data manipulation. The
compiled code is written in C. The package has no runtime dependencies.

## Details

**Threads.** Five functions can use OpenMP:
[`iif`](https://fastverse.org/kit/reference/iif.md),
[`nswitch`](https://fastverse.org/kit/reference/vswitch.md),
[`vswitch`](https://fastverse.org/kit/reference/vswitch.md),
[`psort`](https://fastverse.org/kit/reference/psort.md), and
[`charToFact`](https://fastverse.org/kit/reference/charToFact.md). The
other functions use one thread.

Set the number of threads with `options(kit.nThread = 4L)`. The default
is `1`. If you set a value that is larger than the number of available
processors, kit uses the number of available processors. If kit was
built without OpenMP, all functions use one thread and this option has
no effect.

**Platforms.** kit is tested on Linux, macOS, and Windows. OpenMP is
optional. On macOS, OpenMP requires `libomp` from Homebrew. On Windows,
OpenMP comes with Rtools. See the README for installation details.

**Encodings.** [`psort`](https://fastverse.org/kit/reference/psort.md)
sorts text in the C locale by default. Set `c.locale = FALSE` to use the
session locale.
[`charToFact`](https://fastverse.org/kit/reference/charToFact.md) uses
the session locale. Factor levels and sort order can thus differ between
operating systems for non-ASCII text.
[`vswitch`](https://fastverse.org/kit/reference/vswitch.md) and
[`nswitch`](https://fastverse.org/kit/reference/vswitch.md) convert
mixed encodings to UTF-8 by default.

## See also

[`psum`](https://fastverse.org/kit/reference/psum.md),
[`funique`](https://fastverse.org/kit/reference/funique.md),
[`topn`](https://fastverse.org/kit/reference/topn.md),
[`shareData`](https://fastverse.org/kit/reference/shareData.md)
