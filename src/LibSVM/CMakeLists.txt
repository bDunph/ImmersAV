add_library(LibSVM STATIC libsvm.cpp libsvm.h)
target_include_directories(LibSVM PUBLIC ./)
