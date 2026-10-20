# Tests for funique()/fduplicated()/uniqLen() — unique values and counts — tinytest (see issue #54).
# Migrated from testthat 3e to tinytest to keep kit lean (zero test dependencies).
# Each legacy check("id", actual, expected) maps to expect_identical(..., info="id");
# 31 tolerant cases (identical FALSE but all.equal+typeof TRUE) use expect_kit_equal(..., info="id");
# check(..., error=) maps to expect_error(pattern=, fixed=TRUE, info="id").
# Legacy IDs are preserved as info= labels (prefixed by function for uniqueness).
# Setup and expectations stay interleaved in legacy order: later sections may
# redefine x/y/out_vec/... so setup must not be hoisted above earlier tests.

sys.source("helper-kit.R", envir = environment())
set.seed(123)

x1 = sample(c(1:1000,NA_integer_),1e3,TRUE)
x2 = sample(c(TRUE,NA,FALSE),1e3,TRUE)
x3 = sample(as.numeric(c(1:1000,NA_integer_)),1e3,TRUE)
x4 = sample(as.complex(c(1:1000,NA_complex_,(NaN+0i)/0,NaN)),1e3,TRUE)
x5 = sample(as.character(c(1:1000,NA_integer_)),1e3,TRUE)
x6 = data.frame(a = rep(seq.POSIXt(as.POSIXct("2020-01-01"),as.POSIXct("2020-01-30"),length.out = 5),4L),b = rep(rnorm(5),4L))

  expect_identical(fduplicated(iris$Species), duplicated(iris$Species), info="fduplicated-0015.001")


  expect_identical(fduplicated(iris$Petal.Width), duplicated(iris$Petal.Width), info="fduplicated-0015.002")


  expect_identical(fduplicated(iris$Petal.Length), duplicated(iris$Petal.Length), info="fduplicated-0015.003")


  expect_identical(fduplicated(iris$Sepal.Length), duplicated(iris$Sepal.Length), info="fduplicated-0015.004")


  expect_identical(fduplicated(iris$Sepal.Width), duplicated(iris$Sepal.Width), info="fduplicated-0015.005")


  expect_identical(fduplicated(as.character(iris$Petal.Width)), duplicated(as.character(iris$Petal.Width)), info="fduplicated-0015.006")


  expect_identical(fduplicated(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE)), duplicated(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE)), info="fduplicated-0015.007")


  expect_identical(fduplicated(x1), duplicated(x1), info="fduplicated-0015.008")


  expect_identical(fduplicated(x2), duplicated(x2), info="fduplicated-0015.009")


  expect_identical(fduplicated(x3), duplicated(x3), info="fduplicated-0015.010")


  expect_identical(fduplicated(x4), duplicated(x4), info="fduplicated-0015.011")


  expect_identical(fduplicated(x5), duplicated(x5), info="fduplicated-0015.012")


  expect_identical(fduplicated(data.frame(a=x1,b=x1)), duplicated(data.frame(a=x1,b=x1)), info="fduplicated-0015.013")


  expect_identical(fduplicated(data.frame(a=x2,b=x2)), duplicated(data.frame(a=x2,b=x2)), info="fduplicated-0015.014")


  expect_identical(fduplicated(data.frame(a=x3,b=x3)), duplicated(data.frame(a=x3,b=x3)), info="fduplicated-0015.015")


  expect_identical(fduplicated(data.frame(a=x4,b=x4)), duplicated(data.frame(a=x4,b=x4)), info="fduplicated-0015.016")


  expect_identical(fduplicated(data.frame(a=x5,b=x5)), duplicated(data.frame(a=x5,b=x5)), info="fduplicated-0015.017")


  expect_identical(fduplicated(iris[,5:4]), duplicated(iris[,5:4]), info="fduplicated-0015.018")


  expect_identical(fduplicated(iris[,5:3]), duplicated(iris[,5:3]), info="fduplicated-0015.019")


  expect_identical(fduplicated(iris[,5:2]), duplicated(iris[,5:2]), info="fduplicated-0015.020")


  expect_identical(fduplicated(iris[,5:1]), duplicated(iris[,5:1]), info="fduplicated-0015.021")


  expect_error(fduplicated(raw(4L)), pattern = "Type raw is not supported.", fixed = TRUE, info="fduplicated-0015.022")


  expect_error(fduplicated(iris3), pattern = "Arrays are not yet supported. (please raise a feature request if needed)", fixed = TRUE, info="fduplicated-0015.023")


  expect_identical(fduplicated(matrix(c(1,1,1,1),nrow = 2)), c(FALSE,TRUE), info="fduplicated-0015.024")


  expect_identical(fduplicated(matrix(c(1L,1L,1L,1L),nrow = 2)), c(FALSE,TRUE), info="fduplicated-0015.025")


  expect_identical(fduplicated(matrix(c(TRUE,TRUE,FALSE,FALSE),nrow = 2)), c(FALSE,TRUE), info="fduplicated-0015.026")


  expect_identical(fduplicated(matrix(c("1","1","1","1"),nrow = 2)), c(FALSE,TRUE), info="fduplicated-0015.027")


  expect_identical(fduplicated(matrix(as.complex(c(1,1,1,1)),nrow = 2)), c(FALSE,TRUE), info="fduplicated-0015.028")


  expect_error(fduplicated(matrix(as.raw(c(1,1,1,1)),nrow = 2)), pattern = "Matrix of type raw are not supported.", fixed = TRUE, info="fduplicated-0015.029")


  expect_identical(fduplicated(x6), duplicated(x6), info="fduplicated-0015.030")


  expect_identical(fduplicated(x1, fromLast=TRUE), duplicated(x1, fromLast=TRUE), info="fduplicated-0015.031")


  expect_identical(fduplicated(x2, fromLast=TRUE), duplicated(x2, fromLast=TRUE), info="fduplicated-0015.032")


  expect_identical(fduplicated(x3, fromLast=TRUE), duplicated(x3, fromLast=TRUE), info="fduplicated-0015.033")


  expect_identical(fduplicated(x4, fromLast=TRUE), duplicated(x4, fromLast=TRUE), info="fduplicated-0015.034")


  expect_identical(fduplicated(x5, fromLast=TRUE), duplicated(x5, fromLast=TRUE), info="fduplicated-0015.035")


  expect_identical(fduplicated(matrix(c(1,1,1,1),nrow = 2),fromLast = TRUE), c(TRUE,FALSE), info="fduplicated-0015.036")


  expect_identical(fduplicated(matrix(c(1L,1L,1L,1L),nrow = 2),fromLast = TRUE), c(TRUE,FALSE), info="fduplicated-0015.037")


  expect_identical(fduplicated(matrix(c(TRUE,TRUE,FALSE,FALSE),nrow = 2),fromLast = TRUE), c(TRUE,FALSE), info="fduplicated-0015.038")


  expect_identical(fduplicated(matrix(c("1","1","1","1"),nrow = 2),fromLast = TRUE), c(TRUE,FALSE), info="fduplicated-0015.039")


  expect_identical(fduplicated(matrix(as.complex(c(1,1,1,1)),nrow = 2),fromLast = TRUE), c(TRUE,FALSE), info="fduplicated-0015.040")


  expect_identical(fduplicated(data.frame(a=x1,b=x1),fromLast = TRUE), duplicated(data.frame(a=x1,b=x1),fromLast = TRUE), info="fduplicated-0015.041")


  expect_identical(fduplicated(data.frame(a=x2,b=x2),fromLast = TRUE), duplicated(data.frame(a=x2,b=x2),fromLast = TRUE), info="fduplicated-0015.042")


  expect_identical(fduplicated(data.frame(a=x3,b=x3),fromLast = TRUE), duplicated(data.frame(a=x3,b=x3),fromLast = TRUE), info="fduplicated-0015.043")


  expect_identical(fduplicated(data.frame(a=x4,b=x4),fromLast = TRUE), duplicated(data.frame(a=x4,b=x4),fromLast = TRUE), info="fduplicated-0015.044")


  expect_identical(fduplicated(data.frame(a=x5,b=x5),fromLast = TRUE), duplicated(data.frame(a=x5,b=x5),fromLast = TRUE), info="fduplicated-0015.045")


  expect_identical(fduplicated(iris[,5:4],fromLast = TRUE), duplicated(iris[,5:4],fromLast = TRUE), info="fduplicated-0015.046")

df = iris
df$Petal.Width = as.double(df$Petal.Width)
df$Petal.Length = as.character(df$Petal.Length)
df$Sepal.Width = as.logical(df$Sepal.Width)
df$Sepal.Length = as.complex(df$Sepal.Length)
rdn = sample(c(1,NA_real_,NaN),1e3,TRUE)
x7 = c("UK","USA","FR","IT","IT")
attr(x7,"label") = "Country"
x8 = c(1+1i,1+1i,2+1i,2+1i)
attr(x8,"label") = "complex"
x9 = c(TRUE,TRUE,FALSE,FALSE)
attr(x9,"label") = "logical"
f1 = factor(c("A","B","C","C"))
f2 = factor(c("A","B","C","A"))
f3 = factor(c("A","C","A"),levels = c("A","B","C"))
f4 = factor(c("A",NA,"B","A"))      # NA mixed with levels
f5 = factor(c("B","A",NA,"B"))      # all levels and NA present
f6 = factor(c("A","B"))
attr(f6,"levels") = "A"             # code 2 is now out of range
f7 = structure(c(2L,3L), levels = "A", class = "factor") # two out-of-range codes

  expect_identical(funique(iris$Species), unique(iris$Species), info="funique-0016.001")


  expect_identical(funique(iris$Petal.Width), unique(iris$Petal.Width), info="funique-0016.002")


  expect_identical(funique(iris$Petal.Length), unique(iris$Petal.Length), info="funique-0016.003")


  expect_identical(funique(iris$Sepal.Length), unique(iris$Sepal.Length), info="funique-0016.004")


  expect_identical(funique(iris$Sepal.Width), unique(iris$Sepal.Width), info="funique-0016.005")


  expect_identical(funique(as.character(iris$Petal.Width)), unique(as.character(iris$Petal.Width)), info="funique-0016.006")


  expect_identical(funique(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE)), unique(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE)), info="funique-0016.007")


  expect_identical(funique(x1), unique(x1), info="funique-0016.008")


  expect_identical(funique(x2), unique(x2), info="funique-0016.009")


  expect_identical(funique(x3), unique(x3), info="funique-0016.010")


  expect_identical(funique(x4), unique(x4), info="funique-0016.011")


  expect_identical(funique(x5), unique(x5), info="funique-0016.012")


  expect_identical(funique(data.frame(a=x1,b=x1)), {out = unique(data.frame(a=x1,b=x1)); row.names(out)<-NULL;out}, info="funique-0016.013")


  expect_identical(funique(data.frame(a=x2,b=x2)), {out = unique(data.frame(a=x2,b=x2)); row.names(out)<-NULL;out}, info="funique-0016.014")


  expect_identical(funique(data.frame(a=x3,b=x3)), {out = unique(data.frame(a=x3,b=x3)); row.names(out)<-NULL;out}, info="funique-0016.015")


  expect_identical(funique(data.frame(a=x4,b=x4)), {out = unique(data.frame(a=x4,b=x4)); row.names(out)<-NULL;out}, info="funique-0016.016")


  expect_identical(funique(data.frame(a=x5,b=x5)), {out = unique(data.frame(a=x5,b=x5)); row.names(out)<-NULL;out}, info="funique-0016.017")


  expect_identical(funique(df), {adf = unique(df); row.names(adf) <- NULL; adf }, info="funique-0016.018")


  expect_identical(funique(c(as.Date("2020-05-01"),as.Date("2020-05-01"))), as.Date("2020-05-01"), info="funique-0016.019")


  expect_identical(funique(data.frame(a = c(as.Date("2020-05-01"),as.Date("2020-05-01")), b = c(as.Date("2020-05-01"),as.Date("2020-05-01")))), data.frame(a = c(as.Date("2020-05-01")), b = c(as.Date("2020-05-01"))), info="funique-0016.020")


  expect_identical(funique(matrix(c(1,1,1,1,2,2,3,3,2,2),nrow = 5)), matrix(c(1,1,2,2,3,2),nrow = 3), info="funique-0016.021")


  expect_identical(funique(matrix(as.integer(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), matrix(c(1L,1L,2L,2L,3L,2L),nrow = 3), info="funique-0016.022")


  expect_identical(funique(matrix(c(TRUE,TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,FALSE,FALSE,NA,NA),nrow = 6)), matrix(c(TRUE,FALSE,TRUE,TRUE,FALSE,NA),nrow = 3), info="funique-0016.023")


  expect_identical(funique(matrix(as.character(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), matrix(as.character(c(1,1,2,2,3,2)),nrow = 3), info="funique-0016.024")


  expect_identical(funique(matrix(as.complex(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), matrix(as.complex(c(1,1,2,2,3,2)),nrow = 3), info="funique-0016.025")


  expect_error(funique(matrix(as.raw(c(1,1,1,1)),nrow = 2)), pattern = "Matrix of type raw are not supported.", fixed = TRUE, info="funique-0016.026")


  expect_error(funique(iris3), pattern = "Arrays are not yet supported. (please raise a feature request if needed)", fixed = TRUE, info="funique-0016.027")


  expect_identical(funique(matrix(x1[1:100],ncol=10)), unique(matrix(x1[1:100],ncol=10)), info="funique-0016.028")


  expect_identical(funique(matrix(x2[1:100],ncol=10)), unique(matrix(x2[1:100],ncol=10)), info="funique-0016.029")


  expect_identical(funique(matrix(x3[1:100],ncol=10)), unique(matrix(x3[1:100],ncol=10)), info="funique-0016.030")


  expect_identical(funique(matrix(x4[1:100],ncol=10)), unique(matrix(x4[1:100],ncol=10)), info="funique-0016.031")


  expect_identical(funique(matrix(x5[1:100],ncol=10)), unique(matrix(x5[1:100],ncol=10)), info="funique-0016.032")


  expect_identical(funique(matrix(rdn,ncol=10)), unique(matrix(rdn,ncol=10)), info="funique-0016.033")


  expect_identical(funique(x6), unique(x6), info="funique-0016.034")


  expect_identical(funique(x1, fromLast=TRUE), unique(x1, fromLast=TRUE), info="funique-0016.035")


  expect_identical(funique(x2, fromLast=TRUE), unique(x2, fromLast=TRUE), info="funique-0016.036")


  expect_identical(funique(x3, fromLast=TRUE), unique(x3, fromLast=TRUE), info="funique-0016.037")


  expect_identical(funique(x4, fromLast=TRUE), unique(x4, fromLast=TRUE), info="funique-0016.038")


  expect_identical(funique(x5, fromLast=TRUE), unique(x5, fromLast=TRUE), info="funique-0016.039")


  expect_error(funique(x5, fromLast=NA), pattern = "Argument 'fromLast' must be TRUE or FALSE and length 1.", fixed = TRUE, info="funique-0016.040")


  expect_identical(funique(matrix(x1[1:100],ncol=10),fromLast = TRUE), unique(matrix(x1[1:100],ncol=10),fromLast = TRUE), info="funique-0016.041")


  expect_identical(funique(matrix(x2[1:100],ncol=10),fromLast = TRUE), unique(matrix(x2[1:100],ncol=10),fromLast = TRUE), info="funique-0016.042")


  expect_identical(funique(matrix(x3[1:100],ncol=10),fromLast = TRUE), unique(matrix(x3[1:100],ncol=10),fromLast = TRUE), info="funique-0016.043")


  expect_identical(funique(matrix(x4[1:100],ncol=10),fromLast = TRUE), unique(matrix(x4[1:100],ncol=10),fromLast = TRUE), info="funique-0016.044")


  expect_identical(funique(matrix(x5[1:100],ncol=10),fromLast = TRUE), unique(matrix(x5[1:100],ncol=10),fromLast = TRUE), info="funique-0016.045")


  expect_error(funique(matrix(x5[1:100],ncol=10),fromLast=NA), pattern = "Argument 'fromLast' must be TRUE or FALSE and length 1.", fixed = TRUE, info="funique-0016.046")


  expect_identical(funique(data.frame(a=x1,b=x1),fromLast = TRUE), {out = unique(data.frame(a=x1,b=x1),fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.047")


  expect_identical(funique(data.frame(a=x2,b=x2),fromLast = TRUE), {out = unique(data.frame(a=x2,b=x2),fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.048")


  expect_identical(funique(data.frame(a=x3,b=x3),fromLast = TRUE), {out = unique(data.frame(a=x3,b=x3),fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.049")


  expect_identical(funique(data.frame(a=x4,b=x4),fromLast = TRUE), {out = unique(data.frame(a=x4,b=x4),fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.050")


  expect_identical(funique(data.frame(a=x5,b=x5),fromLast = TRUE), {out = unique(data.frame(a=x5,b=x5),fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.051")


  expect_error(funique(data.frame(a=x5,b=x5),fromLast = NA), pattern = "Argument 'fromLast' must be TRUE or FALSE and length 1.", fixed = TRUE, info="funique-0016.052")


  expect_identical(funique(iris[,5:4],fromLast = TRUE), {out = unique(iris[,5:4],fromLast = TRUE); row.names(out)<-NULL;out}, info="funique-0016.053")


  expect_identical(attr(funique(x7),"label"), "Country", info="attr-0016.054")


  expect_identical(attr(funique(data.frame(a=x7,b=x7,stringsAsFactors = FALSE))[,1],"label"), "Country", info="attr-0016.055")


  expect_identical(attr(funique(x8),"label"), "complex", info="attr-0016.056")


  expect_identical(attr(funique(data.frame(a=x8,b=x8,stringsAsFactors = FALSE))[,1],"label"), "complex", info="attr-0016.057")


  expect_identical(attr(funique(x9),"label"), "logical", info="attr-0016.058")


  expect_identical(attr(funique(data.frame(a=x9,b=x9,stringsAsFactors = FALSE))[,1],"label"), "logical", info="attr-0016.059")


  expect_identical(funique(iris$Species[iris$Species != "setosa"]), unique(iris$Species[iris$Species != "setosa"]), info="funique-0016.060")


  expect_identical(funique(c(FALSE,FALSE,NA,TRUE,NA)), unique(c(FALSE,FALSE,NA,TRUE,NA)), info="funique-0016.061")


  expect_identical(funique(c(FALSE,FALSE,NA,TRUE,NA),fromLast = TRUE), unique(c(FALSE,FALSE,NA,TRUE,NA),fromLast = TRUE), info="funique-0016.062")


  expect_identical(funique(c(TRUE,TRUE,NA,FALSE,NA)), unique(c(TRUE,TRUE,NA,FALSE,NA)), info="funique-0016.063")


  expect_identical(funique(c(TRUE,TRUE,NA,FALSE,NA),fromLast = TRUE), unique(c(TRUE,TRUE,NA,FALSE,NA),fromLast = TRUE), info="funique-0016.064")


  expect_identical(funique(c(NA,NA,FALSE)), unique(c(NA,NA,FALSE)), info="funique-0016.065")


  expect_identical(funique(c(NA,NA,FALSE),fromLast = TRUE), unique(c(NA,NA,FALSE),fromLast = TRUE), info="funique-0016.065-2")


  expect_identical(funique(f1), unique(f1), info="funique-0016.066")


  expect_identical(funique(f2), unique(f2), info="funique-0016.067")


  expect_identical(funique(f3), unique(f3), info="funique-0016.068")


  expect_identical(funique(f1,fromLast = TRUE), unique(f1,fromLast = TRUE), info="funique-0016.069")


  expect_identical(funique(f2,fromLast = TRUE), unique(f2,fromLast = TRUE), info="funique-0016.070")


  expect_identical(funique(f3,fromLast = TRUE), unique(f3,fromLast = TRUE), info="funique-0016.071")


  expect_identical(funique(f4), unique(f4), info="funique-0016.071-2")


  expect_identical(funique(f4,fromLast = TRUE), unique(f4,fromLast = TRUE), info="funique-0016.071-3")


  expect_identical(funique(f5), unique(f5), info="funique-0016.071-4")


  expect_identical(funique(f5,fromLast = TRUE), unique(f5,fromLast = TRUE), info="funique-0016.071-5")


  expect_identical(funique(data.frame(a=c(1,NA,NA,NaN,NaN),b=c(2,2,2,2,2))), {out = unique(data.frame(a=c(1,NA,NA,NaN,NaN),b=c(2,2,2,2,2))); row.names(out)<-NULL;out}, info="funique-0016.072")


  expect_identical(funique(data.frame(a=as.complex(c(1,NA,NA,NaN,NaN)),b=c(2,2,2,2,2))), {out = unique(data.frame(a=as.complex(c(1,NA,NA,NaN,NaN)),b=c(2,2,2,2,2)));row.names(out)<-NULL;out}, info="funique-0016.073")

  # 0-column data.frame reaches dfToMatrix with len_x == 0: must not divide by zero (x86 #DE)
  expect_error(funique(data.frame()), info="funique-0016.074")

  # out-of-range factor codes (e.g. from attr(x,"levels") surgery) are deduplicated by
  # code and returned as NA like base; distinct invalid codes collapse into one NA
  expect_identical(funique(f6), unique(f6), info="funique-0016.075")
  expect_identical(funique(f6,fromLast = TRUE), unique(f6,fromLast = TRUE), info="funique-0016.076")
  expect_identical(funique(structure(5L, levels = "a", class = "factor")), unique(structure(5L, levels = "a", class = "factor")), info="funique-0016.077")
  expect_identical(funique(f7), structure(NA_integer_, levels = "A", class = "factor"), info="funique-0016.078")
  expect_identical(funique(structure(2L, levels = character(0), class = "factor")), unique(structure(2L, levels = character(0), class = "factor")), info="funique-0016.079")

x1 = sample(c(1:1000,NA_integer_),1e4,TRUE)
x2 = sample(as.logical(c(1:1000,NA_integer_)),1e4,TRUE)
x3 = sample(as.numeric(c(1:1000,NA_integer_)),1e4,TRUE)
x4 = sample(as.complex(c(1:1000,NA_complex_,(NaN+0i)/0,NaN)),1e4,TRUE)
x5 = sample(as.character(c(1:1000,NA_integer_)),1e4,TRUE)
df1 = countOccur(x1)
df2 = countOccur(x2)
df3 = countOccur(x3)
df4 = countOccur(x4)
df5 = countOccur(x5)
out = data.frame(unique(iris[,5:4]),Count = as.integer(c(29,7,7,5,1,1,7,10,13,3,7,3,1,5,1,3,5,6,11,3,1,6,3,8,2,1,1)))
row.names(out) = NULL
out2 = mtcars
out2$Count = 1L
out2 = aggregate(out2$Count,by=out2[,10:11],FUN = length)
out2 = out2[order(out2$gear,out2$carb),]
row.names(out2) = NULL
names(out2)[3] = "Count"
out3 = countOccur(mtcars[,10:11])
out3 = out3[order(out3$gear,out3$carb),]
row.names(out3) = NULL

  expect_identical(countOccur(iris$Species)[[2]], c(50L,50L,50L), info="countOccur-0017.000")


  expect_identical(countOccur(as.numeric(iris$Species))[[2]], c(50L,50L,50L), info="countOccur-0017.001")


  expect_identical(countOccur(as.complex(iris$Species))[[2]], c(50L,50L,50L), info="countOccur-0017.002")


  expect_identical(countOccur(as.character(iris$Species))[[2]], c(50L,50L,50L), info="countOccur-0017.003")


  expect_identical(countOccur(c(NA,TRUE,TRUE,FALSE,NA,FALSE))[[2]], c(2L,2L,2L), info="countOccur-0017.004")


  expect_error(countOccur(raw(2L)), pattern = "Type raw is not supported.", fixed = TRUE, info="countOccur-0017.005")


  expect_identical(df1[order(df1$Variable),2], as.vector(table(x1,useNA = "always")), info="0017.006")


  expect_identical(df2[order(df2$Variable),2], as.vector(table(x2,useNA = "always")), info="0017.007")


  expect_identical(df3[order(df3$Variable),2], as.vector(table(x3,useNA = "always")), info="0017.008")


  expect_identical(df4[order(df4$Variable),2], as.vector(table(x4,useNA = "always")), info="0017.009")


  expect_identical(df5[order(df5$Variable),2], as.vector(table(x5,useNA = "always")), info="0017.010")


  expect_identical(countOccur(rep(as.Date("2020-06-02"),10L))[[2]], 10L, info="countOccur-0017.011")


  expect_identical(countOccur(data.frame(a = c(as.Date("2020-05-01"),as.Date("2020-05-01")), b = c(as.Date("2020-05-01"),as.Date("2020-05-01")))), data.frame(a = c(as.Date("2020-05-01")), b = c(as.Date("2020-05-01")), Count = 2L), info="countOccur-0017.012")


  expect_identical(countOccur(iris[,5:4]), out, info="countOccur-0017.013")


  expect_identical(out3, out2, info="0017.014")


  expect_error(countOccur(matrix(c(1,1,1,1),nrow = 2)), pattern = "Array are not yet supported.", fixed = TRUE, info="countOccur-0017.015")

df = iris
df$Petal.Width = as.double(df$Petal.Width)
df$Petal.Length = as.character(df$Petal.Length)
df$Sepal.Width = as.logical(df$Sepal.Width)
df$Sepal.Length = as.complex(df$Sepal.Length)
rdn = sample(c(1,NA_real_,NaN),1e3,TRUE)
x1 = sample(c(1:1000,NA_integer_),1e6,TRUE)
x2 = sample(c(TRUE,NA,FALSE),1e3,TRUE)
x3 = sample(as.numeric(c(1:1000,NA_integer_)),1e3,TRUE)
x4 = sample(as.complex(c(1:1000,NA_complex_,(NaN+0i)/0,NaN)),1e3,TRUE)
x5 = sample(as.character(c(1:1000,NA_integer_)),1e3,TRUE)
x6 = data.frame(a = rep(seq.POSIXt(as.POSIXct("2020-01-01"),as.POSIXct("2020-01-30"),length.out = 5),4L),b = rep(rnorm(5),4L))

  expect_identical(uniqLen(iris$Species), length(unique(iris$Species)), info="uniqLen-0018.001")


  expect_identical(uniqLen(iris$Petal.Width), length(unique(iris$Petal.Width)), info="uniqLen-0018.002")


  expect_identical(uniqLen(iris$Petal.Length), length(unique(iris$Petal.Length)), info="uniqLen-0018.003")


  expect_identical(uniqLen(iris$Sepal.Length), length(unique(iris$Sepal.Length)), info="uniqLen-0018.004")


  expect_identical(uniqLen(iris$Sepal.Width), length(unique(iris$Sepal.Width)), info="uniqLen-0018.005")


  expect_identical(uniqLen(as.character(iris$Petal.Width)), length(unique(as.character(iris$Petal.Width))), info="uniqLen-0018.006")


  expect_identical(uniqLen(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE)), length(unique(c(TRUE,FALSE,TRUE,FALSE,NA,NA,TRUE))), info="uniqLen-0018.007")


  expect_identical(uniqLen(x1), length(unique(x1)), info="uniqLen-0018.008")


  expect_identical(uniqLen(x2), length(unique(x2)), info="uniqLen-0018.009")


  expect_identical(uniqLen(x3), length(unique(x3)), info="uniqLen-0018.010")


  expect_identical(uniqLen(x4), length(unique(x4)), info="uniqLen-0018.011")


  expect_identical(uniqLen(x5), length(unique(x5)), info="uniqLen-0018.012")


  expect_identical(uniqLen(data.frame(a=x1,b=x1)), dim(unique(data.frame(a=x1,b=x1)))[1], info="uniqLen-0018.013")


  expect_identical(uniqLen(data.frame(a=x2,b=x2)), dim(unique(data.frame(a=x2,b=x2)))[1], info="uniqLen-0018.014")


  expect_identical(uniqLen(data.frame(a=x3,b=x3)), dim(unique(data.frame(a=x3,b=x3)))[1], info="uniqLen-0018.015")


  expect_identical(uniqLen(data.frame(a=x4,b=x4)), dim(unique(data.frame(a=x4,b=x4)))[1], info="uniqLen-0018.016")


  expect_identical(uniqLen(data.frame(a=x5,b=x5)), dim(unique(data.frame(a=x5,b=x5)))[1], info="uniqLen-0018.017")


  expect_identical(uniqLen(df), dim(unique(df))[1], info="uniqLen-0018.018")


  expect_identical(uniqLen(c(as.Date("2020-05-01"),as.Date("2020-05-01"))), 1L, info="uniqLen-0018.019")


  expect_identical(uniqLen(data.frame(a = c(as.Date("2020-05-01"),as.Date("2020-05-01")), b = c(as.Date("2020-05-01"),as.Date("2020-05-01")))), 1L, info="uniqLen-0018.020")


  expect_identical(uniqLen(matrix(c(1,1,1,1,2,2,3,3,2,2),nrow = 5)), 3L, info="uniqLen-0018.021")


  expect_identical(uniqLen(matrix(as.integer(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), 3L, info="uniqLen-0018.022")


  expect_identical(uniqLen(matrix(c(TRUE,TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,FALSE,FALSE,NA,NA),nrow = 6)), 3L, info="uniqLen-0018.023")


  expect_identical(uniqLen(matrix(as.character(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), 3L, info="uniqLen-0018.024")


  expect_identical(uniqLen(matrix(as.complex(c(1,1,1,1,2,2,3,3,2,2)),nrow = 5)), 3L, info="uniqLen-0018.025")


  expect_error(uniqLen(matrix(as.raw(c(1,1,1,1)),nrow = 2)), pattern = "Matrix of type raw are not supported.", fixed = TRUE, info="uniqLen-0018.026")


  expect_error(uniqLen(iris3), pattern = "Arrays are not yet supported. (please raise a feature request if needed)", fixed = TRUE, info="uniqLen-0018.027")


  expect_identical(uniqLen(matrix(x1[1:100],ncol=10)), dim(unique(matrix(x1[1:100],ncol=10)))[1], info="uniqLen-0018.028")


  expect_identical(uniqLen(matrix(x2[1:100],ncol=10)), dim(unique(matrix(x2[1:100],ncol=10)))[1], info="uniqLen-0018.029")


  expect_identical(uniqLen(matrix(x3[1:100],ncol=10)), dim(unique(matrix(x3[1:100],ncol=10)))[1], info="uniqLen-0018.030")


  expect_identical(uniqLen(matrix(x4[1:100],ncol=10)), dim(unique(matrix(x4[1:100],ncol=10)))[1], info="uniqLen-0018.031")


  expect_identical(uniqLen(matrix(x5[1:100],ncol=10)), dim(unique(matrix(x5[1:100],ncol=10)))[1], info="uniqLen-0018.032")


  expect_identical(uniqLen(matrix(rdn,ncol=10)), dim(unique(matrix(rdn,ncol=10)))[1], info="uniqLen-0018.033")


  expect_identical(uniqLen(x6), dim(unique(x6))[1], info="uniqLen-0018.034")


  expect_identical(uniqLen(c(TRUE,FALSE,FALSE,FALSE,TRUE)), length(unique(c(TRUE,FALSE,FALSE,FALSE,TRUE))), info="uniqLen-0018.035")


  expect_identical(uniqLen(factor(c("A","C","A"),levels = c("A","B","C"))), 2L, info="uniqLen-0018.036")


  expect_identical(uniqLen(factor(c("A",NA,"B","A"))), length(unique(factor(c("A",NA,"B","A")))), info="uniqLen-0018.036-2")


  expect_identical(uniqLen(factor(c("B","A",NA,"B"))), length(unique(factor(c("B","A",NA,"B")))), info="uniqLen-0018.036-3")


  expect_error(uniqLen(data.frame()), info="uniqLen-0018.037")


  expect_identical(uniqLen(f6), length(unique(f6)), info="uniqLen-0018.038")


  expect_identical(uniqLen(structure(5L, levels = "a", class = "factor")), 1L, info="uniqLen-0018.039")

# issue #74: duplicate tables grow past their initial 256 slots / 192 distinct values.
local({
  set.seed(74)
  n <- 5000L
  xs <- list(
    sample(c(NA_integer_, -1000:1000), n, TRUE),
    sample(c(NA_real_, NaN, 0, runif(2000)), n, TRUE),
    complex(real = sample(c(NA_real_, NaN, runif(1000)), n, TRUE),
            imaginary = sample(c(NA_real_, NaN, runif(1000)), n, TRUE)),
    sample(c(NA_character_, paste0("s", 1:800)), n, TRUE),
    sample(c(NA, TRUE, FALSE), n, TRUE)
  )
  for (x in xs) {
    expect_identical(funique(x), unique(x), info = "funique-0074.grow")
    expect_identical(funique(x, fromLast = TRUE), unique(x, fromLast = TRUE),
                     info = "funique-0074.grow-fromLast")
    expect_identical(fduplicated(x), duplicated(x), info = "fduplicated-0074.grow")
    expect_identical(fduplicated(x, fromLast = TRUE),
                     duplicated(x, fromLast = TRUE), info = "fduplicated-0074.grow-fromLast")
    expect_identical(uniqLen(x), length(unique(x)), info = "uniqLen-0074.grow")
  }
})

# issue #74: matrix/data.frame row tables also start small and grow.
local({
  set.seed(74)
  nr <- 5000L
  nc <- 3L
  xs <- list(
    matrix(sample(c(NA_integer_, -200:200), nr * nc, TRUE), nr, nc),
    matrix(sample(c(0, runif(50)), nr * nc, TRUE), nr, nc),
    matrix(complex(real = sample(round(runif(30), 3), nr * nc, TRUE),
                   imaginary = sample(round(runif(30), 3), nr * nc, TRUE)), nr, nc),
    matrix(sample(c(NA_character_, paste0("s", 1:800)), nr * nc, TRUE), nr, nc),
    matrix(sample(c(NA, TRUE, FALSE), nr * nc, TRUE), nr, nc),
    # >192 distinct rows to grow the logical and complex row tables; NA/NaN
    # entries exercise their rehash normalization.
    unname(as.matrix(expand.grid(rep(list(c(FALSE, TRUE, NA)), 5)))),
    unname(as.matrix(expand.grid(rep(list(c(0+0i, 1+0i, NA_complex_,
                                             complex(real = NaN, imaginary = NaN),
                                             0+1i)), 4))))
  )
  for (x in xs) {
    expect_identical(funique(x), unique(x), info = "funique-0074.grow-matrix")
    expect_identical(funique(x, fromLast = TRUE), unique(x, fromLast = TRUE),
                     info = "funique-0074.grow-matrix-fromLast")
    expect_identical(fduplicated(x), as.vector(duplicated(x)),
                     info = "fduplicated-0074.grow-matrix")
    expect_identical(fduplicated(x, fromLast = TRUE),
                     as.vector(duplicated(x, fromLast = TRUE)),
                     info = "fduplicated-0074.grow-matrix-fromLast")
    expect_identical(uniqLen(x), nrow(unique(x)), info = "uniqLen-0074.grow-matrix")
  }
  df <- data.frame(
    a = sample(c(NA_integer_, -100:100), nr, TRUE),
    b = sample(c(NA_character_, paste0("s", 1:200)), nr, TRUE),
    c = sample(c(NA, TRUE, FALSE), nr, TRUE),
    d = factor(sample(c("x", "y", NA), nr, TRUE)),
    e = sample(c(0, runif(20)), nr, TRUE),
    stringsAsFactors = FALSE
  )
  a <- funique(df); b <- unique(df)
  rownames(a) <- rownames(b) <- NULL
  expect_identical(a, b, info = "funique-0074.grow-df")
  a <- funique(df, fromLast = TRUE); b <- unique(df, fromLast = TRUE)
  rownames(a) <- rownames(b) <- NULL
  expect_identical(a, b, info = "funique-0074.grow-df-fromLast")
  expect_identical(fduplicated(df), duplicated(df),
                   info = "fduplicated-0074.grow-df")
  expect_identical(fduplicated(df, fromLast = TRUE),
                   duplicated(df, fromLast = TRUE),
                   info = "fduplicated-0074.grow-df-fromLast")
  expect_identical(uniqLen(df), nrow(unique(df)), info = "uniqLen-0074.grow-df")
})

