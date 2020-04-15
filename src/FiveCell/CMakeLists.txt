add_library(FiveCell STATIC FiveCell.cpp FiveCell.hpp SoundObject.cpp SoundObject.hpp stb_image.cpp stb_image.h)
target_include_directories(FiveCell PUBLIC ./)
