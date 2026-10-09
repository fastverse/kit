\name{shareData/getData/clearData}
\alias{shareData}
\alias{getData}
\alias{clearData}
\title{ Share Data between R Sessions}
\description{
Experimental functions that enable the user to share a R object between 2 \R sessions.
}
\usage{
shareData(data, map_name, verbose=FALSE)
getData(map_name, verbose=FALSE)
clearData(x, verbose=FALSE)
}
\arguments{
  \item{data}{ A \R object like a vector or a \code{data.frame}.}
  \item{map_name}{ A character. A name for the memory map location where to store the data. Needs to start with a "/" on POSIX platforms, this is done automatically if missing.}
  \item{x}{ An external pointer like the one returned by function \code{shareData}.}
  \item{verbose}{ A logical value \code{TRUE} or \code{FALSE} to provide or not information to the user.}
}
\details{
\code{shareData} is a deep copy, not a pointer: the object is serialized into a raw vector in the sharing session and that raw vector is copied into a shared memory object. \code{getData} copies the raw vector back into the reading session and unserializes it. The full object is thus serialized twice and copied once, which for large objects dominates the cost of the transfer.

The shared object can be read once: \code{getData} unlinks the memory map as soon as it has read the data, so calling \code{getData} again with the same \code{map_name} fails.

The data lives as long as the external pointer returned by \code{shareData} exists in the sharing session: \code{clearData} unmaps and unlinks the shared memory object and returns \code{TRUE}, or \code{FALSE} if it was already cleared. The same cleanup happens automatically when that pointer is garbage collected or the session exits. If the sharing session is killed before any of that, the shared memory object may persist (\code{/dev/shm} on POSIX) until removed.

Functions are currently only tested on POSIX platforms (using \code{shm_open}) and Windows (using named file mappings), other platforms are untested.
}
\value{
\code{shareData} returns a external pointer.
\code{getData} returns an \R object stored in the memory location \code{map_name}.
\code{clearData} returns \code{TRUE} or \code{FALSE} depending on whether the data have been cleared in memory.
}
\note{
Sharing objects is not zero-copy: a plain pointer into the R heap of another session cannot work, because R objects carry process-private metadata (garbage collection, string cache, ALTREP, ...). Issue #43 (\url{https://github.com/fastverse/kit/issues/43}) tracks the request for a lightweight pointer-based alternative.
}
\author{Morgan Jacob}
\examples{
\dontrun{
# In R session 1: share data in memory
x = shareData(mtcars,"share1")

# In R session 2: get data from session 1 (readable only once!)
getData("share1")

# In R session 1: clear data in memory
clearData(x)
}
}
