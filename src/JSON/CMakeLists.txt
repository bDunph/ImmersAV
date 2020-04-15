add_library(JSON STATIC jsoncpp.cpp json.h json-forwards.h)
target_include_directories(JSON PUBLIC ./)
