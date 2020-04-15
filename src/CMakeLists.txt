cmake_minimum_required(VERSION 3.13)

project(avr)

#The version number
set (avr_VERSION_MAJOR 0)
set (avr_VERSION_MINOR 1)
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

	find_library(RAPIDMIX RAPID-MIX_API)
	if(NOT RAPIDMIX)
		message(FATAL_ERROR "Rapid-Mix not found")
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
	"${PROJECT_SOURCE_DIR}/avrConfig.h.in"
	"${PROJECT_BINARY_DIR}/avrConfig.h"
	)

#add the binary tree to the search path for include files
#so that we will find avrConfig.h
if(WIN32)
include_directories(
		
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		${OPENGL_INCLUDE_DIRS}
		"${PROJECT_SOURCE_DIR}/../include"
		"${PROJECT_SOURCE_DIR}/FiveCell"
		"${PROJECT_SOURCE_DIR}/Visual"
		"${PROJECT_SOURCE_DIR}/VR"
		"${PROJECT_SOURCE_DIR}/AvrApp"
		"${PROJECT_SOURCE_DIR}/ValveTools"
		#"${PROJECT_SOURCE_DIR}/Algorithms"
		#"${PROJECT_SOURCE_DIR}/Audio"
	)
elseif(APPLE)
include_directories(

		"/usr/local/include/"
		"/Library/Frameworks/CsoundLib64.framework/Headers/"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/RapidLib/src"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/xmm/src"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/GVF"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/third_party/json"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/Maximilian"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/Maximilian/libs"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/pipo/sdk/src"
		"/Users/bryandunphy/SoftwareAPIs/RAPID-MIX_API/dependencies/pipo/sdk/src/host"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/../include"
		"${PROJECT_SOURCE_DIR}/FiveCell"
		"${PROJECT_SOURCE_DIR}/Visual"
		"${PROJECT_SOURCE_DIR}/VR"
		"${PROJECT_SOURCE_DIR}/AvrApp"
		#"${PROJECT_SOURCE_DIR}/Algorithms"
		#"${PROJECT_SOURCE_DIR}/Audio"
	)

endif()

add_subdirectory(FiveCell)
add_subdirectory(Visual)
add_subdirectory(VR)
add_subdirectory(AvrApp)
if(WIN32)
add_subdirectory(ValveTools)
add_subdirectory(RapidLib)
add_subdirectory(JSON)
add_subdirectory(LibSVM)
endif()
#add_subdirectory(Audio)
#add_subdirectory(Algorithms)

if(APPLE)
	add_executable(avr main.cpp CsoundSession.cpp CsoundSession.hpp lodepng.cpp lodepng.h)
	target_link_libraries(avr AvrApp VR ${OPENVR} Visual FiveCell ${RAPIDMIX} GLEW::GLEW glfw OpenGL::GL ${CSOUND_API} ${CSOUND_PERF_THREAD})
elseif(WIN32)
	add_executable(avr main.cpp CsoundSession.cpp CsoundSession.hpp csPerfThread.cpp csPerfThread.hpp lodepng.cpp lodepng.h)
	target_link_libraries(avr AvrApp VR OpenVR_target ValveTools Visual FiveCell RapidLib JSON LibSVM Glew_target ${GLFW_WIN} ${OPENGL_gl_LIBRARY} Csound_target Libsndfile_target)
endif()
