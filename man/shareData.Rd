\name{shareData/getData/clearData}
\alias{shareData}
\alias{getData}
\alias{clearData}
\title{ Share Data between R Sessions}
\description{
Experimental functions that enable the user to share a R object between 2 \R sessions.
The object is serialized once by \code{shareData} into POSIX shared memory
(or a Windows file mapping). It can then be read multiple times, from the
current session and/or from other processes running as the same user, via
\code{getData}. Reads are non-destructive: the segment lives until the owner
handle returned by \code{shareData} is cleared with \code{clearData} (or
garbage collected). Keep the owner object alive while readers are active.
}
\usage{
shareData(data, map_name, verbose=FALSE)
getData(map_name, verbose=FALSE)
clearData(x, verbose=FALSE)
}
\arguments{
  \item{data}{ A \R object like a vector or a \code{data.frame}.}
  \item{map_name}{ A character. A name for the memory map location where to store the data.}
  \item{x}{ An external pointer like the one returned by function \code{shareData}.}
  \item{verbose}{ A logical value \code{TRUE} or \code{FALSE} to provide or not information to the user.}
}
\value{
\code{shareData} returns a external pointer.
\code{getData} returns an \R object stored in the memory location \code{map_name}. Reads do not consume the segment, so \code{getData} may be called repeatedly and from multiple processes.
\code{clearData} returns \code{TRUE} or \code{FALSE} depending on whether the data have been cleared in memory.
}
\author{Morgan Jacob}
\examples{
# In R session 1: share data in memory
# > x = shareData(mtcars,"share1")
#
# In R session 2: get data from session 1 (repeatable, also in parallel)
# > getData("share1")
# > getData("share1")
#
# In R session 1: clear data in memory once all readers are done
# > clearData(x)
#
# Sharing large immutable globals with background workers (owner stays alive):
# > x = shareData(list(graph = g, dist = D), "/mydata")
# > # workers only receive the short map name and attach via shared memory
# > # mirai::daemons(4)
# > # mirai::mirai_map(ids, function(i) fun(i, kit::getData("/mydata")))
# > # mirai::daemons(0); clearData(x)
}
