# Tests for shareData() / getData() / clearData() — shared-memory data sharing
# tinytest version (see issue #54).
# Legacy IDs 0022.001-0022.002 preserved as info= labels.

# --- R-side validation (issue #59): no shm needed, runs even without shm ---
expect_error(shareData(mtcars, ""), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.001 empty map_name")
expect_error(shareData(mtcars, NA_character_), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.002 NA map_name")
expect_error(shareData(mtcars, character(0)), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.003 length-0 map_name")
expect_error(shareData(mtcars, 123), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.004 non-character map_name")
expect_error(shareData(mtcars, "share59", verbose = NA), pattern = "Argument 'verbose' must be TRUE or FALSE and length 1.", fixed = TRUE, info = "share-0059.005 NA verbose")
expect_error(shareData(mtcars, "share59", verbose = "yes"), pattern = "Argument 'verbose' must be TRUE or FALSE and length 1.", fixed = TRUE, info = "share-0059.006 non-logical verbose")
expect_error(getData(""), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.007 getData empty map_name")
expect_error(getData(NA_character_), pattern = "Argument 'map_name' must be a single non-empty, non-missing string.", fixed = TRUE, info = "share-0059.008 getData NA map_name")
expect_error(getData("share59", verbose = NA), pattern = "Argument 'verbose' must be TRUE or FALSE and length 1.", fixed = TRUE, info = "share-0059.009 getData NA verbose")
expect_error(clearData(1), pattern = "Argument 'x' must be an external pointer like the one returned by shareData().", fixed = TRUE, info = "share-0055.001 clearData invalid pointer")

map_name <- paste0("/kit55-", Sys.getpid())
x <- tryCatch(shareData(mtcars, map_name), error = function(err) NULL)
if (is.null(x)) exit_file("shared memory unavailable - skipping shareData tests")
invisible(gc())
expect_identical(getData(map_name), mtcars, info = "share-0022.001 getData roundtrip")
if (.Platform$OS.type == "windows") {
  expect_identical(getData(map_name), mtcars, info = "share-0055.002 Windows mapping remains readable")
} else {
  expect_error(getData(map_name), pattern = "Creating file mapping...ERROR", fixed = TRUE, info = "share-0055.002 getData consumes map")
}
expect_true(clearData(x), info = "share-0022.002 clearData")
expect_true(!clearData(x), info = "share-0055.003 clearData twice")

x <- tryCatch(shareData(1:10, map_name), error = function(err) NULL)
if (is.null(x)) exit_file("shared memory unavailable - skipping shareData re-share test")
expect_identical(getData(map_name), 1:10, info = "share-0055.004 re-share after clear")
expect_true(clearData(x), info = "share-0055.005 re-share clearData")

size_name <- paste0("/kit55-size-", Sys.getpid())
small <- tryCatch(shareData(1:3, size_name), error = function(err) NULL)
if (is.null(small)) exit_file("shared memory unavailable - skipping shareData size test")
expect_error(shareData(as.list(seq_len(10000)), size_name), pattern = "ERROR", fixed = TRUE, info = "share-0055.006 undersized mapping reuse")
expect_true(clearData(small), info = "share-0055.007 undersized mapping cleanup")

owner_name <- paste0("/kit55-owner-", Sys.getpid())
first <- tryCatch(shareData(1:3, owner_name), error = function(err) NULL)
if (is.null(first)) exit_file("shared memory unavailable - skipping ownership test")
second <- tryCatch(shareData(4:6, owner_name), error = function(err) NULL)
if (is.null(second)) {
  clearData(first)
  exit_file("shared-memory re-share unavailable - skipping ownership test")
}
expect_true(clearData(second), info = "share-0055.008 non-owner cleanup")
expect_identical(getData(owner_name), 4:6, info = "share-0055.009 owner remains readable")
if (.Platform$OS.type != "windows") {
  replacement <- tryCatch(shareData(7:9, owner_name), error = function(err) NULL)
  if (is.null(replacement)) {
    expect_true(clearData(first), info = "share-0055.011 old owner cleanup")
  } else {
    expect_true(clearData(first), info = "share-0055.012 old owner cleanup")
    expect_identical(getData(owner_name), 7:9, info = "share-0055.013 replacement remains readable")
    expect_true(clearData(replacement), info = "share-0055.014 replacement cleanup")
  }
} else {
  expect_true(clearData(first), info = "share-0055.010 owner cleanup")
}
