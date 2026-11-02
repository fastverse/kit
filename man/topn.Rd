\name{topn}
\alias{topn}
\title{ Top N values index}
\description{
  \code{topn} returns the indices (or values) of the \code{n} largest or
  smallest values in a vector. This extends \code{\link{which.max}} and
  \code{\link{which.min}}, which return only the first such index.

  The resulting order is the same as
  \code{order(vec, decreasing = decreasing, na.last = TRUE)[1:n]} when
  \code{hasna} is \code{TRUE} or \code{vec} has no missing values, but
  \code{topn()} avoids sorting the whole input. It is therefore much faster
  when \code{n} is small relative to the input size.
}
\usage{
topn(vec, n=6L, decreasing=TRUE, hasna=TRUE, index=TRUE)
}
\arguments{
  \item{vec}{A numeric or integer vector. Other types are not supported yet.}
  \item{n}{A positive integer greater than or equal to 1.}
  \item{decreasing}{Logical. If \code{TRUE} (default), sort from largest to
  smallest. This is the same as \code{decreasing} in \code{base::order},
  whose default is \code{FALSE}.}
  \item{hasna}{Logical. Whether \code{vec} contains \code{NA} values.
  Default is \code{TRUE}.}
  \item{index}{Logical. If \code{TRUE} (default), return indices. If
  \code{FALSE}, return values.}
}
\details{
  For small \code{n}, \code{topn()} uses memory proportional to \code{n}.
  It falls back to \code{base::order()} when \code{n > 1500} and
  \code{hasna = TRUE}, or when \code{n > 2000} and \code{hasna = FALSE}.
  It also falls back when the selection scan has to rescan too often, for
  example when \code{vec} is already sorted in the opposite direction. The
  fallback allocates a full-length index: about 4 bytes per element. This can
  use much more memory than the fast path.

  For inputs longer than \code{.Machine$integer.max}, \code{topn()} keeps a
  heap of at most \code{n} entries instead, so memory stays proportional to
  \code{n} (about 24 bytes per entry plus 8 bytes per returned index). Unlike
  \code{base::order()}, \code{decreasing = TRUE} works for these inputs. For
  \code{n} above roughly three quarters of the input length this heap can use
  more memory than a full-length \code{order()} index, and \code{order()} is
  faster for \code{decreasing = FALSE}.

  For classed numeric vectors, \code{xtfrm()} is applied once before
  selection, so their order matches \code{order()}/\code{xtfrm()}. This also
  applies to long inputs; a classed long vector whose \code{xtfrm()} result is
  not integer or double errors.
}
\value{
  If \code{index = TRUE}, an integer vector of the indices of the \code{n}
  largest (or smallest) values in \code{vec}. If \code{index = FALSE}, the
  values themselves. For inputs longer than \code{.Machine$integer.max}, the
  indices are returned as a double vector.

  Ties keep their input order. With \code{hasna = TRUE} (default), missing
  values are placed last, in input order. \code{hasna = FALSE} asserts that
  \code{vec} has no missing values: it is not checked, and for unclassed
  vectors \code{NA} is then compared as an ordinary value. For inputs longer
  than \code{.Machine$integer.max}, missing values are always placed last,
  whatever the value of \code{hasna}. For inputs no longer than
  \code{.Machine$integer.max}, the resulting order matches
  \code{order(vec, decreasing = decreasing, method = "radix", na.last = TRUE)[1:n]}
  when \code{vec} has no missing values or \code{hasna = TRUE}.
}
\author{Morgan Jacob}
\examples{
x = rnorm(1e4)

# Example 1: index of top 6 negative values
topn(x, 6L, decreasing=FALSE)
order(x)[1:6]

# Example 2: index of top 6 positive values
topn(x, 6L, decreasing = TRUE)
order(x, decreasing=TRUE)[1:6]

# Example 3: top 6 negative values
topn(x, 6L, decreasing=FALSE, index=FALSE)
sort(x)[1:6]

# Benchmarks
# ----------
# x = rnorm(1e7) # 76Mb
# microbenchmark::microbenchmark(
#   topn=kit::topn(x, 6L),
#   order=order(x, decreasing=TRUE)[1:6],
#   times=10L
# )
# Unit: milliseconds
#  expr min   lq  mean median   uq  max neval
# topn   11   11    13     11   12   18    10
# order 563  565   587    566  602  661    10
#
# microbenchmark::microbenchmark(
#  topn=kit::topn(x, 6L, decreasing=FALSE, index=FALSE),
#  sort=sort(x, partial=1:6)[1:6],
#  times=10L
# )
# Unit: milliseconds
# expr min  lq  mean median   uq  max neval
# topn  11  11    11     11   12   12    10
# sort 167 175   197    178  205  303    10
}
