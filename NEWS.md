# kit 0.0.21 <small>(2026-01-17)</small>

### New Features

- Functions `fpmin`, `fpmax`, and `prange` are added as parallel equivalents of base R `pmin`, `pmax`, and range operations. These functions compute element-wise minimum, maximum, and range (max - min) across multiple vectors. They support integer, logical, and double types, with automatic type promotion. Implemented by @SebKrantz.

### Notes

- The GitHub repo move to [fastverse/kit](https://github.com/fastverse/kit/), a new website exists at [fastverse.org/kit](https://fastverse.org/kit/), and Sebastian Krantz and the *fastverse* Team have taken responsibility to maintain the package. Morgan Jacob has stepped down, but remains available for consultation.

# kit 0.0.20 <small>(2025-04-17)</small>

### Notes

- Update copyright date in c files

- Fix note on CRAN regarding Rf_isFrame

# kit 0.0.19 <small>(2024-09-07)</small>

### Bug Fixes

- Fix multiple warnings in C code.

# kit 0.0.18 <small>(2024-06-06)</small>

### Bug Fixes

- Fix `iif` tests for new version of R.

# kit 0.0.17 <small>(2024-05-03)</small>

### Bug Fixes

- Fix `nswitch`. Thanks to Sebastian Krantz for raising an issue.

### Notes

- Update copyright date in c files

- Fix note on CRAN regarding SETLENGTH

# kit 0.0.16 <small>(2024-03-01)</small>

### Notes

- Check if `"kit.nThread"` is defined before setting it to `1L`

# kit 0.0.15 <small>(2023-10-01)</small>

### Notes

- Correct typo in configure file

# kit 0.0.14 <small>(2023-08-12)</small>

### Notes

- Update configure file to extend support for GCC

- Correct warnings in NEWS.Rd (strong)

- Correct typo in funique.Rd thanks to @davidbudzynski

# kit 0.0.13 <small>(2023-02-24)</small>

### Notes

- Function `pprod` now returns double output even if inputs are integer - in line with `base::prod` - to avoid integer overflows.

- Update configure file

# kit 0.0.12 <small>(2022-10-26)</small>

### New Features

- Function `pcountNA` is equivalent to `pcount(..., value = NA)`.

- Function `pcountNA` and `pcount(..., value = NA)` allow `NA` counting with mixed data type (including `data.frame`). `pcountNA` also supports list-vectors as inputs and counts empty or `NULL` elements as `NA`.

- Functions `panyv`, `panyNA`, `pallv` and `pallNA` are added as efficient wrappers around `pcount` and `pcountNA`. They are parallel equivalents of scalar functions `base::anyNA` and `anyv`, `allv` and `allNA` in the 'collapse' R package.

- Functions `pfirst` and `plast` are added to efficiently obtain the row-wise first and last non-missing value or non-empty element of lists. They are parallel equivalents to the (column-wise) `ffirst` and `flast` functions in the 'collapse' R package. Implemented by @SebKrantz.

- Functions `psum/pprod/pmean` also support logical vectors as input. Implemented by @SebKrantz.

### Bug Fixes

- Function `charToFact` was not returning proper results. Thanks to @alex-raw for raising an issue.

### Notes

- Function `pprod` now returns double output even if inputs are integer - in line with `base::prod` - to avoid integer overflows.

- C compiler warnings on CRAN R-devel caused by compilation with -Wstrict-prototypes are now fixed. Declaration of functions without prototypes is depreciated in all versions of C. Thanks to Sebastian Krantz for the PR.

# kit 0.0.11 <small>(2022-03-19)</small>

### New Features

- Function `pcount` now supports data.frame.

### Bug Fixes

- Function `pcount` now works with specific NA values, i.e. NA_real_, NA_character_ etc...

# kit 0.0.10 <small>(2021-11-28)</small>

### New Features

- Function `psum`, `pmean`, `pprod`, `pany` and `pall` now support lists. Thanks to Sebastian Krantz for the request and code suggestion.

### Bug Fixes

- Function `topn` should now work for ALTREP object. Thanks to @ben-schwen for raising an issue.

# kit 0.0.9 <small>(2021-09-12)</small>

### Notes

- Re-organise header to prevent compilation errors with new version of Clang due to conflicts between R C headers and OpenMP.

# kit 0.0.8 <small>(2021-08-21)</small>

### New Features

- Function `funique` now preserves the attributes if the input is a `data.table`, `tibble` or similar objects. Thanks to Sebastian Krantz for the request.

- Function `topn` now defaults to base R `order` for large value of `n`. Please see updated documentation for more information `?kit::topn`.

- Function `charToFact` gains a new argument `addNA=TRUE` to be used to include (or not) `NA` in levels of the output.

- Function `shareData`, `getData` and `clearData` implemented to share data objects between R sessions. These functions are experimental and might change in the future. Feedback is welcome. Please see `?kit::shareData` for more information.

### Notes

- Few `calloc` functions at C level have been replaced by R C API function `Calloc` to avoid valgrind errors/warnings in Travis CI.

- Errors reported by `rchk` on CRAN have been fixed.

# kit 0.0.7 <small>(2021-03-07)</small>

### New Features

- Function `charToFact` gains a new argument `decreasing=FALSE` to be used to order levels of the output in decreasing or increasing order.

- Function `topn` gains a new argument `index=TRUE` to be used return index (`TRUE`) or values (`FALSE`) of input vector.

### Bug Fixes

- Some tests of memory access errors using valgrind and AddressSanitizer were reported by CRAN. An attempt to fix these errors has been submitted as part of this package version. It also seems that these same errors were causing some tests to fail for `funique` and `psort` on some platforms.

### Notes

- Functions `pmean`, `pprod` and `psum` will result in error if used with factors. Documentation has been updated.

# kit 0.0.6 <small>(2021-02-21)</small>

### New Features

- Function `funique` and `fduplicated` gain an additional argument `fromLast=FALSE` to indicate whether the search should start from the end or beginning [PR#11](https://github.com/fastverse/kit/pull/11).

- Functions `pall`, `pany`, `pmean`, `pprod` and `psum` accept `data.frame` as input [PR#15](https://github.com/fastverse/kit/pull/15). Please see documentation for more information.

- Function `charToFact` is equivalent to to base R `as.factor` but is much quicker and only converts character vector to factor. Note that it is parallelised. For more details and benchmark please see `?kit::charToFact`.

- Function `psort` is experimental and equivalent to to base R `sort` but is only for character vector. It can sort by "C locale" or by "R session locale". For more details and benchmark please see `?kit::psort`.

### Notes

- A few OpenMP directives were missing for functions `vswitch` and `nswitch` for character vectors. These have been added in [PR#12](https://github.com/fastverse/kit/pull/12).

- Function `funique` was not preserving attributes for character, logical and complex vectors/data.frames. Thanks to Sebastian Krantz (@SebKrantz) for bringing that to my attention. This has been fixed in [PR#13](https://github.com/fastverse/kit/pull/13).

- Functions `funique` and `uniqLen` should now be faster for `factor` and `logical` vectors [PR#14](https://github.com/fastverse/kit/pull/14).

# kit 0.0.5 <small>(2020-11-21)</small>

### New Features

- Function `uniqLen(x)` is equivalent to base R `length(unique(x))` and `uniqueN` in package [data.table](https://CRAN.R-project.org/package=data.table). Function `uniqLen`, implemented in C, supports vectors, `data.frame` and `matrix`. It should be faster than these functions. For more details and benchmark please see `?kit::uniqLen`.

- Function `vswitch` now supports mixed encoding and gains an additional argument `checkEnc=TRUE`. Thanks to Xianying Tan (@shrektan) for the request and review [PR#7](https://github.com/fastverse/kit/pull/7).

- Function `nswitch` is a nested version of function `vswitch` and also supports mixed encoding. Please see please see `?kit::nswitch` for further details. Thanks to Xianying Tan (@shrektan) for the request and review [PR#10](https://github.com/fastverse/kit/pull/10).

### Notes

- Small algorithmic improvement for functions `fduplicated`, `funique` and `countOccur` for `vectors`, `data.frame` and `matrix`.

- A tests folder has been added to the source package to track coverage and bugs.

### C-Level Facilities

- Function `nif` has been split into two distinctive functions at C level, one has its arguments evaluated in a lazy way and is for R users and the other one (nifInternalR) is not lazy and is intended for usage at C level.

# kit 0.0.4 <small>(2020-07-21)</small>

### New Features

- Function `countOccur(x)`, implemented in C, is comparable to `base` R function `table`. It returns a `data.frame` and is between 3 to 50 times faster. For more details, please see `?kit::countOccur`.

- Functions `funique` and `fduplicated` now support matrices. Additionally, these two functions should also have better performance compare to previous release.

- Functions `topn` has an additional argument `hasna=TRUE` to indicates whether data contains `NA` value or not. If the data does not contain `NA` values, the function should be faster.

### C-Level Facilities

- A few C functions have been added to subset `data.frame` and `matrix` as well as do other operations. These functions are not exported or visible to the user but might become available and callable at C level in the future.

### Bug Fixes

- Function `fpos` was not properly handling `NaN` and `NA` for complex and double. This should now be fixed. The function has also been changed in case the 'needle' and 'haysatck' are vectors so that a vector is returned.

- Functions `funique` and `fduplicated` were not properly handling data containing `POSIX` data. This has now been fixed.

# kit 0.0.3 <small>(2020-06-21)</small>

### New Features

- Functions `fduplicated(x)` and `funique(x)`, implemented in C, are comparable to `base` R functions `duplicated` and `unique`. For more details, please see `?kit::funique`.

- Functions `psum` and `pprod` have now better performance for type double and complex.

### Bug Fixes

- Function `count(x, y)` now checks that `x` and `y` have the same class and levels. So does `pcount`.

- Function `pmean` was not callable at C level because of a typo. This is now fixed.

# kit 0.0.2 <small>(2020-05-22)</small>

### New Features

- Function `count(x, value)`, implemented in C, to simply count the number of times an element `value` occurs in a vector or in a list `x`. For more details, please see `?kit::count`.

- Function `pmean(..., na.rm=FALSE)`, `pall(..., na.rm=FALSE)`, `pany(..., na.rm=FALSE)` and `pcount(..., value)`, implemented in C, are similar to already available function `psum` and `pprod`. These functions respectively apply base R functions `mean`, `all` and `any` element-wise. For more details, benchmarks and help, please see `?kit::pmean`.

### Bug Fixes

- Fix Solaris Unicode warnings for NEWS file. Benchmarks have been moved from the NEWS file to each function Rd file.

- Fix some `NA` edge cases for `pprod` and `psum` so these functions behave more like base R function `prod` and `sum`.

- Fix installation errors for version of R (<3.5.0).

# kit 0.0.1 <small>(2020-05-03)</small>

### Initial Release

- Function `fpos(needle, haystack, all=TRUE, overlap=TRUE)`, implemented in C, is inspired by base function `which` when used in the following form `which(x == y, arr.ind =TRUE)`. Function `fpos` returns the index(es) or position(s) of a matrix/vector within a larger matrix/vector. Please see `?kit::fpos` for more details.

- Function `iif(test, yes, no, na=NULL, tprom=FALSE, nThread=getOption("kit.nThread"))`, originally contributed as `fifelse` in package [data.table](https://CRAN.R-project.org/package=data.table), was moved to package kit to be developed independently. Unlike the current version of `fifelse`, `iif` allows type promotion like base function `ifelse`. For further details about the differences with `fifelse`, as well as `hutils::if_else` and `dplyr::if_else`, please see `?kit::iif`.

- Function `nif(..., default=NULL)`, implemented in C, is inspired by *SQL CASE WHEN*. It is comparable to [dplyr](https://CRAN.R-project.org/package=dplyr) function `case_when` however it evaluates it arguments in a lazy way (i.e only when needed). Function `nif` was originally contributed as function `fcase` in the [data.table](https://CRAN.R-project.org/package=data.table) package but then moved to package kit so its development may resume independently. Please see `?kit::nif` for more details.

- Function `pprod(..., na.rm=FALSE)` and `psum(..., na.rm=FALSE)`, implemented in C, are inspired by base function `pmin` and `pmax`. These new functions work only for integer, double and complex types and do not recycle vectors. Please see `?kit::psum` for more details.

- Function `setlevels(x, old, new, skip_absent=FALSE)`, implemented in C, may be used to set levels of a factor object. Please see `?kit::setlevels` for more details.

- Function `topn(vec, n=6L, decreasing=TRUE)`, implemented in C, returns the top largest or smallest `n` values for a given numeric vector `vec`. It is inspired by `dplyr::top_n` and equivalent to base functions order and sort in specific cases as shown in the documentation. Please see `?kit::topn` for more details.

- Function `vswitch(x, values, outputs, default=NULL, nThread=getOption("kit.nThread"))`, implemented in C, is a vectorised version of `base` R function `switch`. This function can also be seen as a particular case of function `nif`. Please see `?kit::switch` for more details.

