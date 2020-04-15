add_library(Visual STATIC Graphics.cpp Graphics.hpp ShaderManager.cpp ShaderManager.hpp Log.cpp Log.hpp SystemInfo.cpp SystemInfo.hpp CGLRenderModel.cpp CGLRenderModel.hpp)
target_include_directories(Visual PUBLIC ./)
