#ifndef SYSTEMINFO_HPP 
#define SYSTEMINFO_HPP

#ifdef __APPLE__
#include <GLFW/glfw3.h>
#define vsprintf_s vsnprintf
#elif _WIN32
#include "glfw3.h"
#endif

//#include <GL/glew.h>

#define GLCheckError() GLCheckError_(__FILE__, __LINE__)

void _update_fps_counter(GLFWwindow* window);
void dprintf(const char *fmt, ... );
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam);
float FConvertUint32ToFloat(uint32_t val);
GLenum GLCheckError_(const char *file, int line);

struct ExecutionFlags
	{
		bool flagDebugOpenGL;
		bool flagVBlank;
		bool flagGLFinishHack;			
		bool flagDPrint;
		bool flagDevMode;
	};

#endif
