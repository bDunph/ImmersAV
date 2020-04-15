#include "SystemInfo.hpp"

#include <cstdio>
#include <cstdarg>
#include <string>
#include <iostream>

#ifdef __APPLE__
#define vsprintf_s vsnprintf
#elif _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
#define APIENTRY
#endif

void _update_fps_counter(GLFWwindow* window){

	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if(elapsed_seconds > 0.25){
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf(tmp, "opengl @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );

#ifdef __APPLE__
	vsprintf(buffer, fmt, args);
#elif _WIN32
	vsprintf_s( buffer, fmt, args );
#endif

	va_end( args );

	printf( "%s", buffer );

#ifdef __APPLE__
	std::string charString = buffer;
	std::cout << "Debug Message from dprintf() (SystemInfo.cpp): " << charString << std::endl; 
#elif _WIN32
	OutputDebugStringA( buffer );
#endif

}

//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}

float FConvertUint32ToFloat(uint32_t val)
{
	static_assert(sizeof(float) == sizeof val, "Error: FConvertUint32ToFloat - passed argument is not compatible");
	float returnVal;
	std::memcpy(&returnVal, &val, sizeof(float));
	return returnVal;
}

//-------------------------------------------------
// OpenGL error handling 
//-------------------------------------------------
GLenum GLCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				error = "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				error = "STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			//For some reason this is throwing an undeclared identifier error
			//case GL_INVALID_FRAMEBUFFER_OPERATION:
			//	error = "INVALID_FRAMEBUFFER_OPERATION";
			//	break;
			default:
				std::cout << "Unknown Error" << std::endl;
				break;
		}
		std::cout << "Error: " << error << " | " << file << " (" << line << ")" << std::endl;
	}	
	return errorCode;
}
