# Tests for parallel reductions — psum, pprod, fpmin, fpmax, prange, pall, pany, pmean — tinytest (see issue #54).
# Migrated from testthat 3e to tinytest to keep kit lean (zero test dependencies).
# Each legacy check("id", actual, expected) maps to expect_identical(..., info="id");
# 31 tolerant cases (identical FALSE but all.equal+typeof TRUE) use expect_kit_equal(..., info="id");
# check(..., error=) maps to expect_error(pattern=, fixed=TRUE, info="id").
# Legacy IDs are preserved as info= labels (prefixed by function for uniqueness).
# Setup and expectations stay interleaved in legacy order: later sections may
# redefine x/y/out_vec/... so setup must not be hoisted above earlier tests.

sys.source("helper-kit.R", envir = environment())
set.seed(123)

x = c(1, 3, NA, 5)
y = c(2, NA, 4, 1)
z = c(3, 4, 4, 1)
x0 = rnorm(1000L)
y0 = rnorm(1000L)
z0 = rnorm(1000L)

  expect_identical(psum(x, y, z, na.rm = FALSE), c(6, NA, NA, 7), info="psum-0005.001")


  expect_identical(psum(x, y, z, na.rm = TRUE), c(6, 7, 8, 7), info="psum-0005.002")


  expect_identical(psum(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), c(6L, NA_integer_, NA_integer_, 7L), info="psum-0005.003")


  expect_identical(psum(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), c(6L, 7L, 8L, 7L), info="psum-0005.004")


  expect_error(psum(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical, double and complex types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="psum-0005.005")


  expect_error(psum(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="psum-0005.006")


  expect_error(psum(1:10, 1:5, na.rm = FALSE), pattern = "Argument 2 is of length 5 but argument 1 is of length 10. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="psum-0005.007")


  expect_error(psum(x, as.raw(z), y, na.rm = TRUE), pattern = "Argument 2 is of type raw. Only integer/logical, double and complex types are supported.", fixed = TRUE, info="psum-0005.008")


  expect_identical(psum(1:10, 1:10, 21:30), 1:10 + 1:10 + 21:30, info="psum-0005.009")


  expect_error(psum(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="psum-0005.010")


  expect_identical(psum(x, na.rm = FALSE), x, info="psum-0005.011")


  expect_identical(psum(as.integer(x), y, z, na.rm = TRUE), c(6, 7, 8, 7), info="psum-0005.012")


  expect_identical(psum(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), c(3, 3, 4, 6, 0), info="psum-0005.013")


  expect_identical(psum(x, y, as.integer(z), na.rm = FALSE), c(6, NA, NA, 7), info="psum-0005.014")


  expect_error(psum(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="psum-0005.015")


  expect_identical(psum(x0, y0, z0), x0+y0+z0, info="psum-0005.016")


  expect_identical(psum(as.complex(x0), as.complex(y0), as.complex(z0)), as.complex(x0)+as.complex(y0)+as.complex(z0), info="psum-0005.017")


  expect_identical(psum(as.complex(x0), as.complex(y0), z0), as.complex(x0)+as.complex(y0)+z0, info="psum-0005.018")


  expect_identical(psum(as.complex(x), as.complex(y), as.complex(z), na.rm = FALSE), as.complex(c(6, NA, NA, 7)), info="psum-0005.019")


  expect_identical(psum(as.complex(x), as.complex(y), as.complex(z), na.rm = TRUE), as.complex(c(6, 7, 8, 7)), info="psum-0005.020")


  expect_identical(psum(x, y, z, rep(Inf,4L), na.rm = FALSE), x+y+z+Inf, info="psum-0005.021")


  expect_identical(psum(x, y, z, rep(Inf,4L), na.rm = TRUE), rep(Inf, 4L), info="psum-0005.022")


  expect_identical(psum(NA_integer_, na.rm = TRUE), 0L, info="psum-0005.023")


  expect_identical(psum(NA_real_, na.rm = TRUE), 0, info="psum-0005.024")


  expect_identical(psum(NA_complex_, na.rm = TRUE), 0+0i, info="psum-0005.025")


  expect_identical(psum(iris[,1:2]), rowSums(iris[,1:2]), info="psum-0005.026")


  expect_error(psum(iris[,1:2],iris[,1:2]), pattern = "Argument 1 is of type list. Only integer/logical, double and complex types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="psum-0005.027")


  expect_error(psum(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'psum' is not meaningful for factors.", fixed = TRUE, info="psum-0005.028")


  expect_identical(psum(unclass(mtcars)), psum(mtcars), info="psum-0005.029")

  expect_identical(psum(c(NA_integer_, 1L, NA_integer_), c(2L, NA_integer_, 3L), na.rm = TRUE), c(2L, 1L, 3L), info="psum-0074.001")

  expect_identical(psum(c(NA_real_, 1, NaN), c(2, NA_real_, 3), na.rm = TRUE), c(2, 1, 3), info="psum-0074.002")

  expect_identical(psum(c(NA_complex_, 1+2i, complex(real=NaN, imaginary=3)), c(2+0i, NA_complex_, 4+5i), na.rm = TRUE), c(2+0i, 1+2i, 4+5i), info="psum-0074.003")

  expect_identical(psum(c(Inf, 1), c(-Inf, 2), c(3, NA_real_), na.rm = TRUE), c(NaN, 3), info="psum-0074.004")

  expect_identical(psum(complex(real=c(1, NaN), imaginary=c(2, 3)), complex(real=c(4, 5), imaginary=c(6, NA_real_)), na.rm = TRUE), c(5+8i, 0+0i), info="psum-0074.005")

  psum_named <- c(first=NA_real_, second=1)
  psum_y <- c(first=2, second=3)
  attr(psum_named, "label") <- "first"
  psum_attr <- psum(psum_named, psum_y, na.rm = TRUE)
  psum_expected <- c(first=2, second=4)
  attr(psum_expected, "label") <- "first"
  expect_identical(psum_attr, psum_expected, info="psum-0074.006")
  expect_identical(attr(psum_attr, "label"), "first", info="psum-0074.007")

  expect_identical(psum(c(NA_real_, 1), c(2, 3), na.rm = FALSE), c(NA_real_, 4), info="psum-0074.008")

  expect_identical(writeBin(psum(c(NA_real_, NA_real_), c(-0, -0), na.rm = TRUE), raw(), endian = "little"), as.raw(rep(0, 16)), info="psum-0074.009")


  expect_identical(pprod(x, y, z, na.rm = FALSE), c(6, NA, NA, 5), info="pprod-0006.001")


  expect_identical(pprod(x, y, z, na.rm = TRUE), c(6, 12, 16, 5), info="pprod-0006.002")


  expect_identical(pprod(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), c(6, NA_real_, NA_real_, 5), info="pprod-0006.003")


  expect_identical(pprod(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), c(6, 12, 16, 5), info="pprod-0006.004")


  expect_error(pprod(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical, double and complex types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="pprod-0006.005")


  expect_error(pprod(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pprod-0006.006")


  expect_error(pprod(1:10, 1:5, na.rm = FALSE), pattern = "Argument 2 is of length 5 but argument 1 is of length 10. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pprod-0006.007")


  expect_error(pprod(x, as.raw(z), y, na.rm = TRUE), pattern = "Argument 2 is of type raw. Only integer/logical, double and complex types are supported.", fixed = TRUE, info="pprod-0006.008")


  expect_identical(pprod(1:10, 1:10, 21:30), as.double(1:10 * 1:10 * 21:30), info="pprod-0006.009")


  expect_error(pprod(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="pprod-0006.010")


  expect_identical(pprod(x, na.rm = FALSE), x, info="pprod-0006.011")


  expect_identical(pprod(as.integer(x), y, z, na.rm = TRUE), c(6, 12, 16, 5), info="pprod-0006.012")


  expect_identical(pprod(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), c(2, 3, 4, 5, 1), info="pprod-0006.013")


  expect_identical(pprod(x, y, as.integer(z), na.rm = FALSE), c(6, NA, NA, 5), info="pprod-0006.014")


  expect_error(pprod(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="pprod-0006.015")


  expect_identical(pprod(x0, y0, z0), x0*y0*z0, info="pprod-0006.016")


  expect_identical(pprod(as.complex(x0), as.complex(y0), as.complex(z0)), as.complex(x0)*as.complex(y0)*as.complex(z0), info="pprod-0006.017")


  expect_identical(pprod(as.complex(x0), as.complex(y0), z0), as.complex(x0)*as.complex(y0)*z0, info="pprod-0006.018")


  expect_kit_equal(pprod(as.complex(x), as.complex(y), as.complex(z), na.rm = FALSE), as.complex(c(6, NA, NA, 5)), info="pprod-0006.019")


  expect_identical(pprod(as.complex(x), as.complex(y), as.complex(z), na.rm = TRUE), as.complex(c(6, 12, 16, 5)), info="pprod-0006.020")


  expect_identical(pprod(x, y, z, rep(Inf, 4L), na.rm = FALSE), x*y*z*Inf, info="pprod-0006.021")


  expect_identical(pprod(x, y, z, rep(Inf, 4L), na.rm = TRUE), rep(Inf, 4L), info="pprod-0006.022")


  expect_identical(pprod(NA_integer_, na.rm = TRUE), 1, info="pprod-0006.023")


  expect_identical(pprod(NA_real_, na.rm = TRUE), 1, info="pprod-0006.024")


  expect_identical(pprod(NA_complex_, na.rm = TRUE), 1+0i, info="pprod-0006.025")


  expect_identical(pprod(iris[,1:2]), iris$Sepal.Length*iris$Sepal.Width, info="pprod-0006.026")


  expect_error(pprod(iris[,1:2],iris[,1:2]), pattern = "Argument 1 is of type list. Only integer/logical, double and complex types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="pprod-0006.027")


  expect_error(pprod(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'pprod' is not meaningful for factors.", fixed = TRUE, info="pprod-0006.028")


  expect_identical(pprod(unclass(mtcars)), pprod(mtcars), info="pprod-0006.029")

x = c(1, 3, NA, 5)
y = c(2, NA, 4, 1)
z = c(3, 4, 4, 1)
x0 = rnorm(1000L)
y0 = rnorm(1000L)
z0 = rnorm(1000L)

  expect_identical(fpmin(x, y, z, na.rm = FALSE), c(1, NA, NA, 1), info="fpmin-0008.001")


  expect_identical(fpmin(x, y, z, na.rm = TRUE), c(1, 3, 4, 1), info="fpmin-0008.002")


  expect_identical(fpmin(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), c(1L, NA_integer_, NA_integer_, 1L), info="fpmin-0008.003")


  expect_identical(fpmin(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), c(1L, 3L, 4L, 1L), info="fpmin-0008.004")


  expect_identical(fpmin(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = FALSE), c(FALSE, FALSE, NA), info="fpmin-0008.005")


  expect_identical(fpmin(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = TRUE), c(FALSE, FALSE, FALSE), info="fpmin-0008.006")


  expect_error(fpmin(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical and double types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="fpmin-0008.007")


  expect_error(fpmin(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="fpmin-0008.008")


  expect_error(fpmin(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="fpmin-0008.009")


  expect_identical(fpmin(x, na.rm = FALSE), x, info="fpmin-0008.010")


  expect_identical(fpmin(as.integer(x), y, z, na.rm = TRUE), c(1, 3, 4, 1), info="fpmin-0008.011")


  expect_identical(fpmin(x, y, as.integer(z), na.rm = FALSE), c(1, NA, NA, 1), info="fpmin-0008.012")


  expect_error(fpmin(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="fpmin-0008.013")


  expect_identical(fpmin(x0, y0, z0), pmin(x0, y0, z0), info="fpmin-0008.014")


  expect_identical(fpmin(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), c(1, 3, 4, 1, NA), info="fpmin-0008.015")


  expect_identical(fpmin(NA_integer_, na.rm = TRUE), NA_integer_, info="fpmin-0008.016")


  expect_identical(fpmin(NA_real_, na.rm = TRUE), NA_real_, info="fpmin-0008.017")


  expect_identical(fpmin(iris[,1:2]), pmin(iris$Sepal.Length, iris$Sepal.Width), info="fpmin-0008.018")


  expect_error(fpmin(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'fpmin' is not meaningful for factors.", fixed = TRUE, info="fpmin-0008.019")


  expect_identical(fpmax(x, y, z, na.rm = FALSE), c(3, NA, NA, 5), info="fpmax-0009.001")


  expect_identical(fpmax(x, y, z, na.rm = TRUE), c(3, 4, 4, 5), info="fpmax-0009.002")


  expect_identical(fpmax(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), c(3L, NA_integer_, NA_integer_, 5L), info="fpmax-0009.003")


  expect_identical(fpmax(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), c(3L, 4L, 4L, 5L), info="fpmax-0009.004")


  expect_identical(fpmax(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = FALSE), c(TRUE, TRUE, NA), info="fpmax-0009.005")


  expect_identical(fpmax(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = TRUE), c(TRUE, TRUE, FALSE), info="fpmax-0009.006")


  expect_error(fpmax(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical and double types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="fpmax-0009.007")


  expect_error(fpmax(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="fpmax-0009.008")


  expect_error(fpmax(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="fpmax-0009.009")


  expect_identical(fpmax(x, na.rm = FALSE), x, info="fpmax-0009.010")


  expect_identical(fpmax(as.integer(x), y, z, na.rm = TRUE), c(3, 4, 4, 5), info="fpmax-0009.011")


  expect_identical(fpmax(x, y, as.integer(z), na.rm = FALSE), c(3, NA, NA, 5), info="fpmax-0009.012")


  expect_error(fpmax(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="fpmax-0009.013")


  expect_identical(fpmax(x0, y0, z0), pmax(x0, y0, z0), info="fpmax-0009.014")


  expect_identical(fpmax(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), c(2, 3, 4, 5, NA), info="fpmax-0009.015")


  expect_identical(fpmax(NA_integer_, na.rm = TRUE), NA_integer_, info="fpmax-0009.016")


  expect_identical(fpmax(NA_real_, na.rm = TRUE), NA_real_, info="fpmax-0009.017")


  expect_identical(fpmax(iris[,1:2]), pmax(iris$Sepal.Length, iris$Sepal.Width), info="fpmax-0009.018")


  expect_error(fpmax(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'fpmax' is not meaningful for factors.", fixed = TRUE, info="fpmax-0009.019")


  expect_identical(prange(x, y, z, na.rm = FALSE), c(2, NA, NA, 4), info="prange-0010.001")


  expect_identical(prange(x, y, z, na.rm = TRUE), c(2, 1, 0, 4), info="prange-0010.002")


  expect_identical(prange(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), c(2, NA, NA, 4), info="prange-0010.003")


  expect_identical(prange(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), c(2, 1, 0, 4), info="prange-0010.004")


  expect_identical(prange(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = FALSE), c(1, 1, NA), info="prange-0010.005")


  expect_identical(prange(c(TRUE, FALSE, NA), c(FALSE, TRUE, FALSE), na.rm = TRUE), c(1, 1, 0), info="prange-0010.006")


  expect_error(prange(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical and double types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="prange-0010.007")


  expect_error(prange(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="prange-0010.008")


  expect_error(prange(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="prange-0010.009")


  expect_identical(prange(x, na.rm = FALSE), c(0, 0, NA, 0), info="prange-0010.010")


  expect_identical(prange(as.integer(x), y, z, na.rm = TRUE), c(2, 1, 0, 4), info="prange-0010.011")


  expect_identical(prange(x, y, as.integer(z), na.rm = FALSE), c(2, NA, NA, 4), info="prange-0010.012")


  expect_error(prange(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="prange-0010.013")


  expect_identical(prange(x0, y0, z0), pmax(x0, y0, z0) - pmin(x0, y0, z0), info="prange-0010.014")


  expect_identical(prange(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), c(1, 0, 0, 4, NA), info="prange-0010.015")


  expect_identical(prange(NA_integer_, na.rm = TRUE), NA_real_, info="prange-0010.016")


  expect_identical(prange(NA_real_, na.rm = TRUE), NA_real_, info="prange-0010.017")


  expect_identical(prange(iris[,1:2]), pmax(iris$Sepal.Length, iris$Sepal.Width) - pmin(iris$Sepal.Length, iris$Sepal.Width), info="prange-0010.018")


  expect_error(prange(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'prange' is not meaningful for factors.", fixed = TRUE, info="prange-0010.019")

x = c(TRUE, FALSE, NA, FALSE)
y = c(TRUE, NA, TRUE, TRUE)
z = c(TRUE, TRUE, FALSE, NA)
x0 = sample(c(TRUE, FALSE, NA),1e3,TRUE)
y0 = sample(c(TRUE, FALSE, NA),1e3,TRUE)
z0 = sample(c(TRUE, FALSE, NA),1e3,TRUE)

  expect_identical(pall(x, y, z, na.rm = FALSE), sapply(1:4, function(i) all(x[i],y[i],z[i],na.rm=FALSE)), info="pall-0009.001")


  expect_identical(pall(x, y, z, na.rm = TRUE), sapply(1:4, function(i) all(x[i],y[i],z[i],na.rm=TRUE)), info="pall-0009.002")


  expect_error(pall(x, y, TRUE, na.rm = FALSE), pattern = "Argument 3 is of length 1 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pall-0009.003")


  expect_error(pall(c(TRUE,FALSE,NA), c(TRUE,FALSE), na.rm = FALSE), pattern = "Argument 2 is of length 2 but argument 1 is of length 3. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pall-0009.004")


  expect_error(pall(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="pall-0009.005")


  expect_identical(pall(x, na.rm = FALSE), x, info="pall-0009.006")


  expect_error(pall(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="pall-0009.007")


  expect_error(pall(x, as.integer(z), y, na.rm = TRUE), pattern = "Argument 2 is of type integer. Only logical type is supported.", fixed = TRUE, info="pall-0009.008")


  expect_error(pall(as.double(z), y, na.rm = TRUE), pattern = "Argument 1 is of type double. Only logical type is supported.Data.frame (of logical vectors) is also supported as a single input.", fixed = TRUE, info="pall-0009.009")


  expect_identical(pall(NA, na.rm = TRUE), TRUE, info="pall-0009.010")


  expect_identical(pall(NA, na.rm = FALSE), NA, info="pall-0009.011")


  expect_identical(pall(x0, y0, z0, na.rm = FALSE), sapply(1:1e3, function(i) all(x0[i],y0[i],z0[i],na.rm=FALSE)), info="pall-0009.012")


  expect_identical(pall(x0, y0, z0, na.rm = TRUE), sapply(1:1e3, function(i) all(x0[i],y0[i],z0[i],na.rm=TRUE)), info="pall-0009.013")


  expect_identical(pall(data.frame(x,y), na.rm = FALSE), pall(x,y, na.rm = FALSE), info="pall-0009.014")


  expect_identical(pany(x, y, z, na.rm = FALSE), sapply(1:4, function(i) any(x[i],y[i],z[i],na.rm=FALSE)), info="pany-0010.001")


  expect_identical(pany(x, y, z, na.rm = TRUE), sapply(1:4, function(i) any(x[i],y[i],z[i],na.rm=TRUE)), info="pany-0010.002")


  expect_error(pany(x, y, TRUE, na.rm = FALSE), pattern = "Argument 3 is of length 1 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pany-0010.003")


  expect_error(pany(c(TRUE,FALSE,NA), c(TRUE,FALSE), na.rm = FALSE), pattern = "Argument 2 is of length 2 but argument 1 is of length 3. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pany-0010.004")


  expect_error(pany(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="pany-0010.005")


  expect_identical(pany(x, na.rm = FALSE), x, info="pany-0010.006")


  expect_error(pany(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="pany-0010.007")


  expect_error(pany(x, as.integer(z), y, na.rm = TRUE), pattern = "Argument 2 is of type integer. Only logical type is supported.", fixed = TRUE, info="pany-0010.008")


  expect_error(pany(as.double(z), y, na.rm = TRUE), pattern = "Argument 1 is of type double. Only logical type is supported.Data.frame (of logical vectors) is also supported as a single input.", fixed = TRUE, info="pany-0010.009")


  expect_identical(pany(NA, na.rm = TRUE), TRUE, info="pany-0010.010")


  expect_identical(pany(NA, na.rm = FALSE), NA, info="pany-0010.011")


  expect_identical(pany(x0, y0, z0, na.rm = FALSE), sapply(1:1e3, function(i) any(x0[i],y0[i],z0[i],na.rm=FALSE)), info="pany-0010.012")


  expect_identical(pany(x0, y0, z0, na.rm = TRUE), sapply(1:1e3, function(i) any(x0[i],y0[i],z0[i],na.rm=TRUE)), info="pany-0010.013")


  expect_identical(pany(data.frame(x,y), na.rm = FALSE), pany(x,y, na.rm = FALSE), info="pany-0010.014")

x = c(1, 3, NA, 5)
y = c(2, NA, 4, 1)
z = c(3, 4, 4, 1)
x0 = rnorm(100L)
y0 = rnorm(100L)
z0 = rnorm(100L)
x1 = sample(c(1,2,NA),1e2,TRUE)
y1 = sample(c(1,2,NA),1e2,TRUE)
z1 = sample(c(1,2,NA),1e2,TRUE)

  expect_identical(pmean(x, y, z, na.rm = FALSE), sapply(1:4, function(i) mean(c(x[i], y[i], z[i]), na.rm = FALSE)), info="pmean-0011.001")


  expect_identical(pmean(x, y, z, na.rm = TRUE), sapply(1:4, function(i) mean(c(x[i], y[i], z[i]), na.rm = TRUE)), info="pmean-0011.002")


  expect_error(pmean(as.raw(z), y, na.rm = TRUE), pattern = "Argument 1 is of type raw. Only integer/logical and double types are supported. A data.frame (of the previous types) is also supported as a single input.", fixed = TRUE, info="pmean-0011.003")


  expect_error(pmean(x, y, 1:2, na.rm = FALSE), pattern = "Argument 3 is of length 2 but argument 1 is of length 4. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pmean-0011.004")


  expect_error(pmean(1:10, 1:5, na.rm = FALSE), pattern = "Argument 2 is of length 5 but argument 1 is of length 10. If you wish to 'recycle' your argument, please use rep() to make this intent clear to the readers of your code.", fixed = TRUE, info="pmean-0011.005")


  expect_error(pmean(x, as.raw(z), y, na.rm = TRUE), pattern = "Argument 2 is of type raw. Only integer/logical and double types are supported.", fixed = TRUE, info="pmean-0011.006")


  expect_error(pmean(x, y, z, na.rm = NA), pattern = "Argument 'na.rm' must be TRUE or FALSE and length 1.", fixed = TRUE, info="pmean-0011.007")


  expect_identical(pmean(x, na.rm = FALSE), sapply(1:4, function(i) mean(c(x[i]), na.rm = FALSE)), info="pmean-0011.008")


  expect_identical(pmean(c(1,3,NA,5,NA), c(2,NA,4,1,NA), na.rm = TRUE), sapply(1:5, function(i) mean(c(c(1,3,NA,5,NA)[i], c(2,NA,4,1,NA)[i]), na.rm = TRUE)), info="pmean-0011.009")


  expect_error(pmean(na.rm = FALSE), pattern = "Please supply at least 1 argument. (0 argument supplied)", fixed = TRUE, info="pmean-0011.010")


  expect_kit_equal(pmean(x0, y0, z0), sapply(1:100, function(i) mean(c(x0[i], y0[i], z0[i]), na.rm = FALSE)), info="pmean-0011.011")


  expect_identical(pmean(x, y, z, rep(Inf,4L), na.rm = FALSE), sapply(1:4, function(i) mean(c(x[i], y[i], z[i],rep(Inf,4L)[i]), na.rm = FALSE)), info="pmean-0011.012")


  expect_identical(pmean(x, y, z, rep(Inf,4L), na.rm = TRUE), sapply(1:4, function(i) mean(c(x[i], y[i], z[i],rep(Inf,4L)[i]), na.rm = TRUE)), info="pmean-0011.013")


  expect_identical(pmean(as.integer(x), as.integer(y), as.integer(z), na.rm = FALSE), sapply(1:4, function(i) mean(c(as.integer(x[i]), as.integer(y[i]), as.integer(z[i])), na.rm = FALSE)), info="pmean-0011.014")


  expect_identical(pmean(as.integer(x), as.integer(y), as.integer(z), na.rm = TRUE), sapply(1:4, function(i) mean(c(as.integer(x[i]), as.integer(y[i]), as.integer(z[i])), na.rm = TRUE)), info="pmean-0011.015")


  expect_identical(pmean(as.integer(x), y, z, na.rm = TRUE), sapply(1:4, function(i) mean(c(as.integer(x[i]), y[i], z[i]), na.rm = TRUE)), info="pmean-0011.016")


  expect_identical(pmean(x, y, as.integer(z), na.rm = FALSE), sapply(1:4, function(i) mean(c(x[i], y[i], as.integer(z[i])), na.rm = FALSE)), info="pmean-0011.017")


  expect_identical(pmean(NA_integer_, na.rm = FALSE), mean(NA_integer_,na.rm = FALSE), info="pmean-0011.018")


  expect_identical(pmean(NA_real_, na.rm = FALSE), mean(NA_real_,na.rm = FALSE), info="pmean-0011.019")


  expect_kit_equal(pmean(x0, y0, z0, na.rm = TRUE), sapply(1:100, function(i) mean(c(x0[i], y0[i], z0[i]), na.rm = TRUE)), info="pmean-0011.020")


  expect_identical(pmean(x1, y1, z1, na.rm = FALSE), sapply(1:100, function(i) mean(c(x1[i], y1[i], z1[i]), na.rm = FALSE)), info="pmean-0011.021")


  expect_identical(pmean(x1, y1, z1, na.rm = TRUE), sapply(1:100, function(i) mean(c(x1[i], y1[i], z1[i]), na.rm = TRUE)), info="pmean-0011.022")


  expect_identical(pmean(NA_integer_, na.rm = TRUE), mean(NA_integer_,na.rm = TRUE), info="pmean-0011.023")


  expect_identical(pmean(NA_real_, na.rm = TRUE), mean(NA_real_,na.rm = TRUE), info="pmean-0011.024")


  expect_identical(pmean(data.frame(x,y,z), na.rm = TRUE), pmean(x,y,z,na.rm = TRUE), info="pmean-0011.025")


  expect_error(pmean(1:150,iris$Species, na.rm = FALSE), pattern = "Function 'pmean' is not meaningful for factors.", fixed = TRUE, info="pmean-0011.026")


  expect_identical(pmean(unclass(mtcars)), pmean(mtcars), info="pmean-0011.027")

