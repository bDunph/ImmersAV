add_library(Studio STATIC Studio.cpp Studio.hpp stb_image.cpp stb_image.h)
target_include_directories(Studio PUBLIC ./)
