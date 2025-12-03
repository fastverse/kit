# Introduction to kit

``` r
library(kit)
#> Attaching kit 0.0.20 (OPENMP enabled using 1 thread)
```

## Overview

**kit** provides a collection of fast utility functions implemented in C
for data manipulation in R. It serves as a lightweight, high-performance
toolkit for tasks that are either slow or cumbersome in base R, such as
row-wise operations, vectorized conditionals, and duplicate detection.

Key features include:

- **Parallel statistical functions**: Row-wise operations (`psum`,
  `pmean`, `pfirst`) using OpenMP.
- **Vectorized conditionals**: Fast `if-else` logic (`iif`, `nif`,
  `vswitch`) that preserves attributes.
- **Efficient set operations**: Faster `unique`, `duplicated`, and
  `count` for vectors and data frames.
- **Partial sorting**: Retrieve top N elements without sorting the
  entire vector (`topn`).
- **Factor utilities**: Fast character-to-factor conversion
  (`charToFact`) and level manipulation (`setlevels`).

Most functions are implemented in C and support multi-threading where
applicable, making them significantly faster than their base R
equivalents on large datasets.

## Parallel Statistical Functions

Computing row-wise statistics across multiple vectors or data frame
columns is a common task. While base R has
[`pmin()`](https://rdrr.io/r/base/Extremes.html) and
[`pmax()`](https://rdrr.io/r/base/Extremes.html), it lacks efficient
equivalents for sum, mean, or product. **kit** fills this gap.

### Row-wise Arithmetic

[`psum()`](https://fastverse.github.io/kit/reference/psum.md),
[`pmean()`](https://fastverse.github.io/kit/reference/psum.md), and
[`pprod()`](https://fastverse.github.io/kit/reference/psum.md) compute
parallel sum, mean, and product respectively. They accept multiple
vectors or a single list/data frame.

``` r
x <- c(1, 3, NA, 5)
y <- c(2, NA, 4, 1)
z <- c(3, 4, 4, 1)

# Parallel sum
psum(x, y, z, na.rm = TRUE)
#> [1] 6 7 8 7

# Parallel mean
pmean(x, y, z, na.rm = TRUE)
#> [1] 2.000000 3.500000 4.000000 2.333333
```

They are particularly useful for data frames:

``` r
df <- data.frame(a = c(1, 2, 3), b = c(4, 5, 6), c = c(7, 8, 9))
psum(df)
#> [1] 12 15 18
```

### Coalescing Values

[`pfirst()`](https://fastverse.github.io/kit/reference/psum.md) and
[`plast()`](https://fastverse.github.io/kit/reference/psum.md) return
the first or last non-missing value across a set of vectors. This is
equivalent to the SQL `COALESCE` function (for `pfirst`).

``` r
primary   <- c(NA, 2, NA, 4)
secondary <- c(1, NA, 3, NA)
fallback  <- c(0, 0, 0, 0)

# Take first available value
pfirst(primary, secondary, fallback)
#> [1] 1 2 3 4
```

### Logical and Count Operations

You can check for conditions or count values row-wise with `pall`,
`pany`, and `pcount`.

``` r
a <- c(TRUE, FALSE, NA, TRUE)
b <- c(TRUE, NA, TRUE, FALSE)
c <- c(NA, TRUE, FALSE, TRUE)

# Any TRUE per row?
pany(a, b, c, na.rm = TRUE)
#> [1] TRUE TRUE TRUE TRUE

# Count NAs per row
pcountNA(a, b, c)
#> [1] 1 1 1 0

# Count specific value (e.g., TRUE) per row
pcount(a, b, c, value = TRUE)
#> [1] 2 1 1 2
```

## Vectorized Conditionals

### Fast If-Else (`iif`)

Base R’s [`ifelse()`](https://rdrr.io/r/base/ifelse.html) is known to be
slow and often strips attributes (like `Date` class or factor levels).
[`iif()`](https://fastverse.github.io/kit/reference/iif.md) is a faster,
more robust alternative that preserves attributes from the `yes`
argument.

``` r
dates <- as.Date(c("2024-01-01", "2024-01-02", "2024-01-03"))

# Base ifelse strips class
class(ifelse(dates > "2024-01-01", dates, dates - 1))
#> [1] "numeric"

# iif preserves class
class(iif(dates > "2024-01-01", dates, dates - 1))
#> [1] "Date"
```

It also supports explicit `NA` handling:

``` r
x <- c(-2, -1, NA, 1, 2)
iif(x > 0, "positive", "non-positive", na = "missing")
#> [1] "non-positive" "non-positive" "missing"      "positive"     "positive"
```

### Nested Conditionals (`nif`)

For multiple conditions,
[`nif()`](https://fastverse.github.io/kit/reference/nif.md) offers a
cleaner, more efficient syntax than nested
[`ifelse()`](https://rdrr.io/r/base/ifelse.html) calls, similar to SQL’s
`CASE WHEN`.

``` r
score <- c(95, 82, 67, 45, 78)

nif(
  score >= 90, "A",
  score >= 80, "B", 
  score >= 70, "C",
  score >= 60, "D",
  default = "F"
)
#> [1] "A" "B" "D" "F" "C"
```

### Vectorized Switch (`vswitch`, `nswitch`)

[`vswitch()`](https://fastverse.github.io/kit/reference/vswitch.md) maps
input values to outputs efficiently.

``` r
status_code <- c(1L, 2L, 3L, 1L, 4L)

vswitch(
  x = status_code,
  values = c(1L, 2L, 3L),
  outputs = c("pending", "approved", "rejected"),
  default = "unknown"
)
#> [1] "pending"  "approved" "rejected" "pending"  "unknown"
```

For pairwise syntax,
[`nswitch()`](https://fastverse.github.io/kit/reference/vswitch.md)
pairs values and outputs directly.

``` r
nswitch(status_code,
  1L, "pending",
  2L, "approved", 
  3L, "rejected",
  default = "unknown"
)
#> [1] "pending"  "approved" "rejected" "pending"  "unknown"
```

It can also replace with values from other vectors (columns), mixing
scalars and vectors:

``` r
df <- data.frame(
  code = c(1, 2, 1, 3, 2),
  val_a = c(10, 20, 30, 40, 50),
  val_b = c(100, 200, 300, 400, 500)
)
with(df, nswitch(code,
  1, val_a,
  2, val_b,
  3, 0,
  default = NA_real_
))
#> [1]  10 200  30   0 500
```

## Fast Unique and Duplicates

**kit** provides optimized versions of
[`unique()`](https://rdrr.io/r/base/unique.html) and
[`duplicated()`](https://rdrr.io/r/base/duplicated.html) that are
significantly faster for vectors and data frames.

### Unique Values and Duplicates

``` r
vec <- c("a", "b", "a", "c", "b")

# Get unique values
funique(vec)
#> [1] "a" "b" "c"

# Check for duplicates
fduplicated(vec)
#> [1] FALSE FALSE  TRUE FALSE  TRUE
```

[`uniqLen()`](https://fastverse.github.io/kit/reference/funique.md)
efficiently counts the number of unique elements without allocating the
unique vector itself:

``` r
df <- data.frame(
  x = c(1, 1, 2, 2),
  y = c("a", "a", "b", "b")
)
uniqLen(df)
#> [1] 2
funique(df)
#>   x y
#> 1 1 a
#> 2 2 b
```

### Counting Occurrences

[`countOccur()`](https://fastverse.github.io/kit/reference/count.md)
produces a frequency table (similar to
[`table()`](https://rdrr.io/r/base/table.html) or `dplyr::count()`) but
returns a standard data frame.

``` r
countOccur(c("apple", "banana", "apple", "cherry"))
#>   Variable Count
#> 1    apple     2
#> 2   banana     1
#> 3   cherry     1
```

## Sorting and Utilities

### Partial Sorting (`topn`)

Sorting a large vector just to get the top few elements is inefficient.
[`topn()`](https://fastverse.github.io/kit/reference/topn.md) uses a
partial sorting algorithm to retrieve the top (or bottom) N indices or
values.

``` r
set.seed(42)
x <- rnorm(1000)

# Get indices of top 5 values
topn(x, n = 5)
#> [1] 988 525 820 459 900

# Get the actual values (decreasing = FALSE for bottom values)
topn(x, n = 5, decreasing = FALSE, index = FALSE)
#> [1] -3.371739 -3.017933 -2.993090 -2.958780 -2.699930
```

### Factor Manipulation

[`charToFact()`](https://fastverse.github.io/kit/reference/charToFact.md)
is a fast alternative to
[`as.factor()`](https://rdrr.io/r/base/factor.html) for character
vectors, with control over `NA` levels.

``` r
charToFact(c("a", "b", NA, "a"))
#> [1] a    b    <NA> a   
#> Levels: a b <NA>
```

[`setlevels()`](https://fastverse.github.io/kit/reference/setlevels.md)
allows you to change factor levels by reference (in-place), avoiding
object copying.

### Finding Positions (`fpos`)

[`fpos()`](https://fastverse.github.io/kit/reference/fpos.md) finds the
positions of a pattern (needle) within a vector (haystack). It can be
used to find occurrences of one vector inside another.

``` r
haystack <- c(1, 2, 3, 4, 1, 2, 5)
needle <- c(1, 2)

fpos(needle, haystack)
#> [1] 1 5
```

## Summary

| Task               | kit function                                                              | Base R equivalent                                         |
|:-------------------|:--------------------------------------------------------------------------|:----------------------------------------------------------|
| **Row-wise sum**   | [`psum()`](https://fastverse.github.io/kit/reference/psum.md)             | `rowSums(cbind(...))`                                     |
| **Row-wise mean**  | [`pmean()`](https://fastverse.github.io/kit/reference/psum.md)            | `rowMeans(cbind(...))`                                    |
| **First non-NA**   | [`pfirst()`](https://fastverse.github.io/kit/reference/psum.md)           | `apply(..., 1, function(x) x[!is.na(x)][1])`              |
| **Fast if-else**   | [`iif()`](https://fastverse.github.io/kit/reference/iif.md)               | [`ifelse()`](https://rdrr.io/r/base/ifelse.html)          |
| **Nested if-else** | [`nif()`](https://fastverse.github.io/kit/reference/nif.md)               | Nested [`ifelse()`](https://rdrr.io/r/base/ifelse.html)   |
| **Switch**         | [`vswitch()`](https://fastverse.github.io/kit/reference/vswitch.md)       | [`match()`](https://rdrr.io/r/base/match.html) + indexing |
| **Unique values**  | [`funique()`](https://fastverse.github.io/kit/reference/funique.md)       | [`unique()`](https://rdrr.io/r/base/unique.html)          |
| **Top N indices**  | [`topn()`](https://fastverse.github.io/kit/reference/topn.md)             | `order()[1:n]`                                            |
| **Char to Factor** | [`charToFact()`](https://fastverse.github.io/kit/reference/charToFact.md) | [`as.factor()`](https://rdrr.io/r/base/factor.html)       |

For comprehensive details and performance benchmarks, please refer to
the individual function documentation.
