cmake_minimum_required(VERSION 3.13)

project(ImmersAV)

#The version number
set (immersAV_VERSION_MAJOR 0)
set (immersAV_VERSION_MINOR 1)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL 4.1 REQUIRED)

if(APPLE)

	find_package(glfw3 REQUIRED)
	find_package(GLEW REQUIRED)
	
	find_library(CSOUND_API CsoundLib64)
	if(NOT CSOUND_API)
		message(FATAL_ERROR "CSound not found")
	endif()

	find_library(CSOUND_PERF_THREAD csnd6.6.0)
	if(NOT CSOUND_PERF_THREAD)
		message(FATAL_ERROR "CSound Performance Thread not found")
	endif()
	
	find_library(OPENVR OpenVR)
	if(NOT OPENVR)
		message(FATAL_ERROR "OpenVR not found")
	endif()

elseif(WIN32)

	find_library(GLEW_WIN glew32 "${PROJECT_SOURCE_DIR}/../lib")
	if(NOT GLEW_WIN)
		message(FATAL_ERROR "Glew not found")
	endif()
	add_library(Glew_target SHARED IMPORTED)
	set_property(TARGET Glew_target PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/glew32.dll")
	set_property(TARGET Glew_target PROPERTY IMPORTED_IMPLIB ${GLEW_WIN})

	find_library(GLFW_WIN glfw3 "${PROJECT_SOURCE_DIR}/../lib")
	if(NOT GLFW_WIN)
		message(FATAL_ERROR "Glfw not found")
	endif()

	find_library(CSOUND_LIB csound64 "${PROJECT_SOURCE_DIR}/../lib") 
	if(NOT CSOUND_LIB)
		message(FATAL_ERROR "Csound.lib not found")
	endif()
	add_library(Csound_target SHARED IMPORTED)
	set_property(TARGET Csound_target PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/csound64.dll")
	set_property(TARGET Csound_target PROPERTY IMPORTED_IMPLIB ${CSOUND_LIB})

	find_library(LIB_SND_FILE libsndfile-1 "${PROJECT_SOURCE_DIR}/../lib")
	if(NOT LIB_SND_FILE)
		message(FATAL_ERROR "libsndfile not found")
	endif()
	add_library(Libsndfile_target SHARED IMPORTED)
	set_property(TARGET Libsndfile_target PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/libsndfile-1.dll")
	set_property(TARGET Libsndfile_target PROPERTY IMPORTED_IMPLIB ${LIB_SND_FILE})

	find_library(OPENVR_LIB openvr_api "${PROJECT_SOURCE_DIR}/../lib")
	if(NOT OPENVR_LIB)
		message(FATAL_ERROR "OpenVR not found")
	endif()
	add_library(OpenVR_target SHARED IMPORTED)
	set_property(TARGET OpenVR_target PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/openvr_api.dll")
	set_property(TARGET OpenVR_target PROPERTY IMPORTED_IMPLIB ${OPENVR_LIB})

endif()


#configure a header file to pass some of the CMake settings
#to the source code
configure_file(
	"${PROJECT_SOURCE_DIR}/immersAVConfig.h.in"
	"${PROJECT_BINARY_DIR}/immersAVConfig.h"
	)

#add the binary tree to the search path for include files
#so that we will find immersAVConfig.h
if(WIN32)
include_directories(
		
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		${OPENGL_INCLUDE_DIRS}
		"${PROJECT_SOURCE_DIR}/../include"
		"${PROJECT_SOURCE_DIR}/Studio"
		"${PROJECT_SOURCE_DIR}/Visual"
		"${PROJECT_SOURCE_DIR}/VR"
		"${PROJECT_SOURCE_DIR}/ImmersAVApp"
		"${PROJECT_SOURCE_DIR}/ValveTools"
	)
elseif(APPLE)
include_directories(

		"/usr/local/include/"
		"/Library/Frameworks/CsoundLib64.framework/Headers/"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/../include"
		"${PROJECT_SOURCE_DIR}/Studio"
		"${PROJECT_SOURCE_DIR}/Visual"
		"${PROJECT_SOURCE_DIR}/VR"
		"${PROJECT_SOURCE_DIR}/AvrApp"
	)

endif()

add_subdirectory(Studio)
add_subdirectory(Visual)
add_subdirectory(VR)
add_subdirectory(ImmersAVApp)
add_subdirectory(RapidLib)
add_subdirectory(JSON)
add_subdirectory(LibSVM)
if(WIN32)
add_subdirectory(ValveTools)
endif()

if(APPLE)
	add_executable(immersAV main.cpp CsoundSession.cpp CsoundSession.hpp lodepng.cpp lodepng.h)
	target_link_libraries(immersAV ImmersAVApp VR ${OPENVR} Visual Studio RapidLib JSON LibSVM GLEW::GLEW glfw OpenGL::GL ${CSOUND_API} ${CSOUND_PERF_THREAD})
elseif(WIN32)
	add_executable(immersAV main.cpp CsoundSession.cpp CsoundSession.hpp csPerfThread.cpp csPerfThread.hpp lodepng.cpp lodepng.h)
	target_link_libraries(immersAV ImmersAVApp VR OpenVR_target ValveTools Visual Studio RapidLib JSON LibSVM Glew_target ${GLFW_WIN} ${OPENGL_gl_LIBRARY} Csound_target Libsndfile_target)
endif()
