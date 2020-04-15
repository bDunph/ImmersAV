#include <vector>

#ifdef _WIN32 
#include <windows.h>
#endif

#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>

#include "lodepng.h"
#include "Graphics.hpp"
#include "ShaderManager.hpp"
#include "Log.hpp"
#include "SystemInfo.hpp"

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

#define PI 3.14159265

//******** TERRIBLE GLOBAL VARIABLES HAVE TO FIX THIS WHEN I GET A CHANCE**********//
bool m_bFirstMouse = true;
double m_dLastX = 320.0f;
double m_dLastY = 160.0f;
float m_fYaw = 0.0f;
float m_fPitch = 0.0f;	
glm::vec3 m_vec3DevCamFront;

//-----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
Graphics::Graphics(std::unique_ptr<ExecutionFlags>& flagPtr) : 
	m_pGLContext(nullptr),
	m_nCompanionWindowWidth(640),
	m_nCompanionWindowHeight(480),
	m_iTrackedControllerCount(0),
	m_iTrackedControllerCount_Last(-1),
	m_iValidPoseCount_Last(-1),
	m_unControllerVAO(0),
	m_glControllerVertBuffer(0),
	m_unSceneVAO(0),
	m_glMainShaderProgramID(0),
	m_unCompanionWindowProgramID(0),
	m_unControllerTransformProgramID(0),
	m_unRenderModelProgramID(0),	
	m_nControllerMatrixLocation(-1),
	m_nRenderModelMatrixLocation(-1),
	m_bVblank(true),
	m_bGLFinishHack(true),
	m_bDebugOpenGL(false),
	m_unImageCount(0),
	m_fDeltaTime(0.0),
	m_fLastFrame(0.0),
	m_bPBOFirstFrame(true),
	//m_bWriteInProgress(false),
	m_bPboIndex(false)
	//m_uiFrameNumber(0)
{
	m_bDebugOpenGL = flagPtr->flagDebugOpenGL;
	m_bVblank = flagPtr->flagVBlank;
	m_bGLFinishHack = flagPtr->flagGLFinishHack;
	m_bDebugPrintMessages = flagPtr->flagDPrint;	
	m_bDevMode = flagPtr->flagDevMode;

	m_pRotationVal = std::make_unique<int>();
	*m_pRotationVal = 0;

	m_bRecordScreen = false;

	//m_tStartTime = time(0);

	machineLearning.bRandomParams = false;
	machineLearning.bRecord = false;
	machineLearning.bTrainModel = false;
	machineLearning.bRunModel = false;
	machineLearning.bSaveTrainingData = false;
	machineLearning.bHaltModel = false;
	machineLearning.bLoadTrainingData = false;
}

//----------------------------------------------------------------------
// Initialise OpenGL context, companion window, glew and vsync.
// ---------------------------------------------------------------------	
bool Graphics::BInitGL(bool fullscreen)
{
	
	//start gl context and O/S window using the glfw helper library
	if(!glfwInit())
	{
		std::cerr << "ERROR: could not start GLFW3\n";
		return false;
	}

	

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	if(m_bDebugOpenGL && !m_bDevMode)
	{
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	if(!fullscreen)
	{
		windowWidth = m_nCompanionWindowWidth;
		windowHeight = m_nCompanionWindowHeight;
	} else 
	{
		GLFWmonitor* mon = glfwGetPrimaryMonitor();
		const GLFWvidmode* vmode = glfwGetVideoMode(mon);

		windowWidth = vmode->width;
		windowHeight = vmode->height;
		m_nCompanionWindowWidth = windowWidth;
		m_nCompanionWindowHeight = windowHeight;
	}

	m_pGLContext = glfwCreateWindow(windowWidth, windowHeight, "AVR", NULL, NULL);	

	if(!m_pGLContext)
	{
		std::cerr << "ERROR: could not open window with GLFW3\n";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_pGLContext);
	
	//setup for mouse camera control by disabling the cursor while the program is running 
	glfwSetInputMode(m_pGLContext, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	// Call glGetError() to clear glewInit() error bug
	//glGetError();

#ifdef _WIN32
	if(m_bVblank)
	{
		//turn on vsync on windows
		//it uses the WGL_EXT_swap_control extension
		typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if(!wglSwapIntervalEXT)
		{
			std::cout << "Warning: VSync not enabled" << std::endl;
			m_bVblank = false;
		}
		wglSwapIntervalEXT(1);
		std::cout << "Graphics::BInitGL - VSync Enabled" << std::endl;
	}
#endif
	const GLubyte* renderer = glGetString(GL_RENDERER); //get renderer string
	const GLubyte* version = glGetString(GL_VERSION); //version as a string
	std::cout << "Graphics: " << renderer << std::endl;
	std::cout << "OpenGL version supported " << version << std::endl;

	//tell GL only to draw onto a pixel if a shape is closer to the viewer
	glEnable(GL_DEPTH_TEST);//enable depth testing
	glDepthFunc(GL_LESS);//depth testing interprets a smaller value as 'closer'
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mengerShaderProg = BCreateSceneShaders("kifs_sierpinski");
	if(mengerShaderProg == NULL)
	{
		std::cout << "mengerShaderProg returned NULL: Graphics::BInitGL" << std::endl;
		return false;
	}

	std::string csdFileName = "avr_mandelGrain.csd";
	if(!fiveCell.setup(csdFileName)) 
	{
		std::cout << "fiveCell setup failed: Graphics BInitGL" << std::endl;
		return false;
	}

	if(!fiveCell.BSetupRaymarchQuad(mengerShaderProg))
	{
		std::cout << "raymarch quad failed setup: Graphics::BInitGL" << std::endl;
		return false;
	}

	//create matrices for devmode
	if(m_bDevMode)
	{
		m_fFov = 45.0f;
		m_matDevProjMatrix = glm::perspective(m_fFov, (float)m_nCompanionWindowWidth/ (float)m_nCompanionWindowHeight, 0.1f, 10000.0f);
		float r = m_nCompanionWindowWidth * 0.5f;
		float t = m_nCompanionWindowHeight * 0.5f;
		
		//variables for view matrix
		m_vec3DevCamPos = glm::vec3(0.0f, 1.0f, -1.0f);	
		m_vec3DevCamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		m_vec3DevCamFront = glm::vec3(0.0f, 0.0f, -1.0f);

		//values for framebuffer setup to make up for no headset
		m_nRenderWidth = m_nCompanionWindowWidth;
		m_nRenderHeight = m_nCompanionWindowHeight; 
	}

	m_fMaxDist = 21.0f;
	m_vec3InitCamPos = glm::vec3(0.0f);

	return true;
}

//-----------------------------------------------------------------------------
// Create shaders for scene geometry
//----------------------------------------------------------------------------
GLuint Graphics::BCreateSceneShaders(std::string shaderName){

	std::string vertName = shaderName;
	std::string fragName = shaderName;
	std::string vertShaderName = vertName.append(".vert");
	std::string fragShaderName = fragName.append(".frag");

	//load shaders
	std::string vertShader;
	bool isVertLoaded = load_shader(vertShaderName.c_str(), vertShader);
	if(!isVertLoaded) return NULL;
	
	std::string fragShader;
	bool isFragLoaded = load_shader(fragShaderName.c_str(), fragShader);
	if(!isFragLoaded) return NULL;
	
	const char* vertCString = vertShader.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertCString, NULL);
	glCompileShader(vs);
	//delete[] vertShader;
	//check for compile errors
	bool isVertCompiled = shader_compile_check(vs);
	if(!isVertCompiled) return NULL;
	
	const char* fragCString = fragShader.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragCString, NULL);
	glCompileShader(fs);
	//delete[] fragShader;
	//check for compile errors
	bool isFragCompiled = shader_compile_check(fs);
	if(!isFragCompiled) return NULL;
	
	GLuint shaderProg = glCreateProgram();
	glAttachShader(shaderProg, fs);
	glAttachShader(shaderProg, vs);
	glLinkProgram(shaderProg);
	bool didShadersLink = shader_link_check(shaderProg);
	if(!didShadersLink) return NULL;

	return shaderProg;
}

//-----------------------------------------------------------------------------
// Creates shaders used for the controllers 
//-----------------------------------------------------------------------------
bool Graphics::BCreateDefaultShaders()
{
	if(!m_bDevMode){
		m_unControllerTransformProgramID = CompileGLShader(
			"Controller",

			// vertex shader
			"#version 410\n"
			"uniform mat4 matrix;\n"
			"layout(location = 0) in vec4 position;\n"
			"layout(location = 1) in vec3 v3ColorIn;\n"
			"out vec4 v4Color;\n"
			"void main()\n"
			"{\n"
			"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
			"	gl_Position = matrix * position;\n"
			"}\n",

			// fragment shader
			"#version 410\n"
			"in vec4 v4Color;\n"
			"layout(location = 0) out vec4 outputColor;\n"
			"void main()\n"
			"{\n"
			"   outputColor = v4Color;\n"
			"}\n"
			);
		m_nControllerMatrixLocation = glGetUniformLocation(m_unControllerTransformProgramID, "matrix");
		if(m_nControllerMatrixLocation == -1)
		{
			std::cout << "Unable to find matrix uniform in controller shader\n" << std::endl;;
			return false;
		}

		m_unRenderModelProgramID = CompileGLShader( 
			"render model",

			// vertex shader
			"#version 410\n"
			"uniform mat4 matrix;\n"
			"layout(location = 0) in vec4 position;\n"
			"layout(location = 1) in vec3 v3NormalIn;\n"
			"layout(location = 2) in vec2 v2TexCoordsIn;\n"
			"out vec2 v2TexCoord;\n"
			"void main()\n"
			"{\n"
			"	v2TexCoord = v2TexCoordsIn;\n"
			"	gl_Position = matrix * vec4(position.xyz, 1);\n"
			"}\n",

			//fragment shader
			"#version 410 core\n"
			"uniform sampler2D diffuse;\n"
			"in vec2 v2TexCoord;\n"
			"layout(location = 0) out vec4 outputColor;\n"
			"void main()\n"
			"{\n"
			"   outputColor = texture( diffuse, v2TexCoord);\n"
			"}\n"

			);
		m_nRenderModelMatrixLocation = glGetUniformLocation( m_unRenderModelProgramID, "matrix" );
		if(m_nRenderModelMatrixLocation == -1)
		{
			std::cout << "Unable to find matrix uniform in render model shader\n" << std::endl;
			return false;
		}
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"	
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
		);

	if(!m_bDevMode){
		return m_unControllerTransformProgramID != 0
			&& m_unRenderModelProgramID != 0
			&& m_unCompanionWindowProgramID != 0;
	} 

	return m_unCompanionWindowProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason. ***********************************************************************Need to converge this code with BMainShaderSetup*************************
//-----------------------------------------------------------------------------
GLuint Graphics::CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		dprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Graphics::BSetupStereoRenderTargets(std::unique_ptr<VR_Manager>& vrm)
{
	
	if (!m_bDevMode && vrm != nullptr)
	{
		vrm->m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );
	} 
	else if (!m_bDevMode && vrm == nullptr)
	{
		return false;
	}

	//float* dummyTex = new float[m_nRenderWidth * m_nRenderHeight * sizeof(float)];
	//for(int i = 0; i < (m_nRenderWidth * m_nRenderHeight * sizeof(float)); i++)
	//{
	//	dummyTex[i] = 0.4f;
	//}
	//std::cout << "DummyTex val : " << dummyTex[340] << std::endl;
	//glGetError();
	//test texture for pbo
	//glGenTextures(1, &m_gluiDummyTexture);
	//GLCheckError();
	//glBindTexture(GL_TEXTURE_2D, m_gluiDummyTexture);
	//GLCheckError();
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//GLCheckError();
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//GLCheckError();
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nRenderWidth, m_nRenderHeight, 0, GL_RED, GL_FLOAT, dummyTex);
	//GLCheckError();
	//glBindTexture(GL_TEXTURE_2D, 0);
	//GLCheckError();

	// Create temporary FBO to hold 2D data texture
	bool done = BCreateStorageFBO();
	if(!done) return false;

	// Create Pixel buffer object to asynchronously read back from gpu
	CreatePBO();

	bool fboL = BCreateFrameBuffer(leftEyeDesc, true);

	if(!fboL) return false;

	if(!m_bDevMode)
	{
		bool fboR = BCreateFrameBuffer(rightEyeDesc, false);

		if(!fboR) return false;
	}

	m_vec4TranslationVal = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	//delete[] dummyTex;

	return true;
}

//-----------------------------------------------------------------------------
// Create a pixel buffer object to read back data from GPU to CPU
//-----------------------------------------------------------------------------
void Graphics::CreatePBO()
{
	glGetError();

	for(int i = 0; i < 2; i++)
	{
		glGenBuffers(1, &pbo[i]);
		if(m_bDebugOpenGL && m_bDevMode)
		{
			GLCheckError();
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
		if(m_bDebugOpenGL && m_bDevMode)
		{
			GLCheckError();
		}

		glBufferData(GL_PIXEL_PACK_BUFFER, m_nRenderWidth * m_nRenderHeight * 4 * sizeof(GLubyte), NULL, GL_STREAM_READ);	
		if(m_bDebugOpenGL && m_bDevMode)
		{
			GLCheckError();
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		if(m_bDebugOpenGL && m_bDevMode)
		{
			GLCheckError();
		}
	}
	
}

//-----------------------------------------------------------------------------
// Create framebuffer to act as a destination for a blit operation from 
// gluiDataTexture2DMulti (GL_TEXTURE_2D_MULTISAMPLE) to gluiDataTexture2D 
// (GL_TEXTURE_2D)
//-----------------------------------------------------------------------------
bool Graphics::BCreateStorageFBO()
{
	glGenFramebuffers(1, &m_gluiStorageFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gluiStorageFBO);

	glGenTextures(1, &m_gluiDataTexture2D);		
	glBindTexture(GL_TEXTURE_2D, m_gluiDataTexture2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gluiDataTexture2D, 0);

	buf[0] = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, buf);

 	//check FBO status before setting up m_nResolveFramebufferId
	//TODO: put this status test in its own function in systemInfo.cpp
	auto status1 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status1 != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: m_gluiStorageFBO not created : " << std::to_string(status1) << " -- Graphics::BCreateStorageFBO" << std::endl;

		if(status1 == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT){
			std::cout << "ERROR: Incomplete Attachment" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_UNDEFINED){
			std::cout << "ERROR: Undefined" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT){
			std::cout << "ERROR: Incomplete Missing Attachment" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER){
			std::cout << "ERROR: Incomplete Draw Buffer" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER){
			std::cout << "ERROR: Incomplete Read Buffer" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_UNSUPPORTED){
			std::cout << "ERROR: Unsupported" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE){
			std::cout << "ERROR: Incomplete Multisample" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS){
			std::cout << "ERROR: Incomplete Layer Targets" << std::endl;
		}

		return false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

//-----------------------------------------------------------------------------
// Creates two frame buffers. One with a depth and colour attachments. Another with just a   
// colour attachment. Returns true if the buffers were set up.
// Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool Graphics::BCreateFrameBuffer(FramebufferDesc& framebufferDesc, bool leftEye)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, m_nRenderWidth, m_nRenderHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	if(leftEye)
	{
		//create a texture to hold data from shader and attach to gl_color_attachment1
		glGenTextures(1, &framebufferDesc.m_gluiDataTexture2DMulti);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_gluiDataTexture2DMulti);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, true); 
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_gluiDataTexture2DMulti, 0);

		// set up the two textures as draw buffers
		buffers[0] = GL_COLOR_ATTACHMENT0;
		buffers[1] = GL_COLOR_ATTACHMENT1;
		glDrawBuffers(2, buffers);

	}
	else if(!leftEye)
	{
		rightBuffer[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, rightBuffer);
	}
	
	//check FBO status before setting up m_nResolveFramebufferId
	auto status1 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status1 != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: m_nRenderFramebuffer not created : " << std::to_string(status1) << " -- Graphics::BCreateFrameBuffer" << std::endl;

		if(status1 == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT){
			std::cout << "ERROR: Incomplete Attachment" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_UNDEFINED){
			std::cout << "ERROR: Undefined" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT){
			std::cout << "ERROR: Incomplete Missing Attachment" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER){
			std::cout << "ERROR: Incomplete Draw Buffer" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER){
			std::cout << "ERROR: Incomplete Read Buffer" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_UNSUPPORTED){
			std::cout << "ERROR: Unsupported" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE){
			std::cout << "ERROR: Incomplete Multisample" << std::endl;
		} else if(status1 == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS){
			std::cout << "ERROR: Incomplete Layer Targets" << std::endl;
		}

		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	auto status2 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status2 != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: m_nResolveFramebuffer not created : " << std::to_string(status2) << " -- Graphics::BCreateFrameBuffer" << std::endl;
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Graphics::BSetupCompanionWindow()
{
	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, -1), glm::vec2(0, 0)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, -1), glm::vec2(1, 0)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, 1), glm::vec2(0, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, 1), glm::vec2(1, 1)) );

	// right eye verts
	//vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(0, 0)) );
	//vVerts.push_back( VertexDataWindow( Vector2(1, -1), Vector2(1, 0)) );
	//vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(0, 1)) );
	//vVerts.push_back( VertexDataWindow( Vector2(1, 1), Vector2(1, 1)) );

	//GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
	GLushort vIndices[] = { 0, 1, 3,  0, 3, 2};
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays( 1, &m_unCompanionWindowVAO );
	glBindVertexArray( m_unCompanionWindowVAO );

	glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

//to keep track of mouse movement
void DevMouseCallback(GLFWwindow* window, double xpos, double ypos){

	if(m_bFirstMouse) // this bool variable is initially set to true
	{
    		m_dLastX = xpos;
    		m_dLastY = ypos;
    		m_bFirstMouse = false;
	}
	
	float xOffset = xpos - m_dLastX;
	float yOffset = m_dLastY - ypos;
	m_dLastX = xpos;
	m_dLastY = ypos;
	
	float sensitivity = 0.05f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;	
	
	m_fYaw += xOffset;
	m_fPitch += yOffset;
	
	if(m_fPitch > 89.9f)
  		m_fPitch =  89.9f;
	if(m_fPitch < -89.9f)
  		m_fPitch = -89.9f; 

	glm::vec3 front;
    	front.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    	front.y = sin(glm::radians(m_fPitch));
    	front.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    	m_vec3DevCamFront = glm::normalize(front);	
}

//***********************************************************************************************************
// Get Info from Vive Controllers
// **********************************************************************************************************
void Graphics::GetControllerEvents(std::unique_ptr<VR_Manager>& vrm){

	machineLearning.bRandomParams = vrm->m_bViveRandParams;
	machineLearning.bRecord = vrm->m_bViveRecordTrainingExample;
	machineLearning.bTrainModel = vrm->m_bViveTrainModel;
	machineLearning.bRunModel = vrm->m_bViveRunModel;
	machineLearning.bSaveModel = vrm->m_bViveSaveModel;
	machineLearning.bLoadModel = vrm->m_bViveLoadModel;

	m_vVRPos = vrm->m_vMoveCam;
}
// **********************************************************************************************************

void Graphics::DevProcessInput(GLFWwindow *window){
	
	float cameraSpeed = 5.0f * m_fDeltaTime; // adjust accordingly
	
	//convert camera position to spherical coordinates
	//float radius = glm::length(m_vec3DevCamPos);
	//float theta = atan2(m_vec3DevCamPos.z, m_vec3DevCamPos.x);
	//float phi = acos(m_vec3DevCamPos.y / radius);

	//std::cout << radius << std::endl;

	////clamp radius between 0 and m_fMaxDist
	//radius = std::clamp(radius, 0.0f, m_fMaxDist);

	////convert back to cartesian coordinates 
	//m_vec3DevCamPos.x = cos(theta) * cos(phi) * radius;
	//m_vec3DevCamPos.z = sin(theta) * cos(phi) * radius;
	//m_vec3DevCamPos.y = sin(phi) * radius;

    	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        	m_vec3DevCamPos += cameraSpeed * m_vec3DevCamFront;
    	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        	m_vec3DevCamPos -= cameraSpeed * m_vec3DevCamFront;
    	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        	m_vec3DevCamPos -= glm::normalize(glm::cross(m_vec3DevCamFront, m_vec3DevCamUp)) * cameraSpeed;
    	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        	m_vec3DevCamPos += glm::normalize(glm::cross(m_vec3DevCamFront, m_vec3DevCamUp)) * cameraSpeed;	
	
	//keep camera movement on the XZ plane
	if(m_vec3DevCamPos.y < 1.0f || m_vec3DevCamPos.y > 1.0f) m_vec3DevCamPos.y = 1.0f;

	//keep camera within a certain distance from origin
	if(m_vec3DevCamPos.x > m_fMaxDist) m_vec3DevCamPos.x = m_fMaxDist;
	if(m_vec3DevCamPos.x < -m_fMaxDist) m_vec3DevCamPos.x = -m_fMaxDist;
	if(m_vec3DevCamPos.z > m_fMaxDist) m_vec3DevCamPos.z = m_fMaxDist;
	if(m_vec3DevCamPos.z < -m_fMaxDist) m_vec3DevCamPos.z = -m_fMaxDist;
	
	//record data
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_R) == GLFW_REPEAT){
		machineLearning.bRecord = true;
		//std::cout << "RECORD ON" << std::endl;
	} 

	//randomise parameters
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		machineLearning.bRandomParams = true;
		//std::cout << "RANDOM" << std::endl;
	} else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
		machineLearning.bRandomParams = false;
	}

	//train model
	if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
		machineLearning.bTrainModel = true;
	} else if(glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE){
		machineLearning.bTrainModel = false;
	}

	//run model
	if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){
		machineLearning.bRunModel = true;
		machineLearning.bHaltModel = false;
	}

	//halt model
	if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
		machineLearning.bHaltModel = true;
		machineLearning.bRunModel = false;
	}

	//save model
	if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
		machineLearning.bSaveTrainingData = true;
	} else if(glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE){
		machineLearning.bSaveTrainingData = false;
	}

	//load model
	if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
		machineLearning.bLoadTrainingData = true;
	} else if(glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE){
		machineLearning.bLoadTrainingData = false;
	}
}

//-----------------------------------------------------------------------------
// Read data back from data texture on the gpu
//-----------------------------------------------------------------------------
void Graphics::TransferDataToCPU()
{
	// clear error flag
	glGetError();

	// Bind PBO that was written to in the previous frame
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[!m_bPboIndex]);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	m_structPboInfo.pboPtr = new unsigned char[m_nRenderWidth * m_nRenderHeight * 4];

	void* pboData = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	// copy memory block from pixel buffer object to memory block on cpu
	memcpy(m_structPboInfo.pboPtr, pboData, m_nRenderWidth * m_nRenderHeight * 4 * sizeof(unsigned char));

	//std::cout << "PBO : " << static_cast<unsigned>(m_pDataSize[8]) << std::endl;

	if(glUnmapBuffer(GL_PIXEL_PACK_BUFFER) != GL_TRUE)
		std::cout << "ERROR: PBO failed to unmap: Graphics::TransferDataToCPU()" << std::endl;

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

}

//-----------------------------------------------------------------------------
// Update eye independent values before scene is rendered
//-----------------------------------------------------------------------------
void Graphics::UpdateSceneData(std::unique_ptr<VR_Manager>& vrm)
{

	glm::vec3 cameraPosition;

	if(!m_bDevMode && !m_bPBOFirstFrame) //m_bPBOFirstFrame here because rightDirVec4 was nan on 1st frame
	{
		m_mat4CurrentViewMatrix = vrm->GetCurrentViewMatrix();
		cameraPosition = glm::vec3(m_mat4CurrentViewMatrix[3][0], m_mat4CurrentViewMatrix[3][1], m_mat4CurrentViewMatrix[3][2]);

		//*** VR MOVEMENT CONTROLS

		glm::mat4 invMat = glm::inverse(m_mat4CurrentViewMatrix);
		glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 rightVec = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec4 rightVec4 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 forwardVec = glm::cross(rightVec, upVec);
		glm::vec4 forwardVec4 = glm::vec4(forwardVec.x, forwardVec.y, forwardVec.z, 0.0);
		glm::vec4 forwardDirVec4 = invMat * forwardVec4;
		forwardDirVec4 = glm::normalize(forwardDirVec4);
		glm::vec4 rightDirVec4 = invMat * rightVec4;
		rightDirVec4 = glm::normalize(rightDirVec4);

		float camSpeed = 0.8f * m_fDeltaTime; // adjust accordingly
		float translationMag = glm::length(m_vec4TranslationVal);

		if (m_vVRPos.x > 0.0f)
		{
        		m_vec4TranslationVal -= camSpeed * rightDirVec4;
			if(translationMag >= 30.0f) m_vec4TranslationVal += camSpeed * rightDirVec4; 
		}
    		if (m_vVRPos.x < 0.0f)
		{
        		m_vec4TranslationVal += camSpeed * rightDirVec4;
			if(translationMag >= 30.0f) m_vec4TranslationVal -= camSpeed * rightDirVec4;
		}
    		if (m_vVRPos.y > 0.0f)
		{
        		m_vec4TranslationVal += camSpeed * forwardDirVec4; 
			if(translationMag >= 30.0f) m_vec4TranslationVal -= camSpeed * forwardDirVec4;
		}
    		if (m_vVRPos.y < 0.0f)
		{
        		m_vec4TranslationVal -= camSpeed * forwardDirVec4;
			if(translationMag >= 30.0f) m_vec4TranslationVal += camSpeed * forwardDirVec4;
		}
			
		////keep camera movement on the XZ plane
		//if(cameraPosition.y < 1.0f || cameraPosition.y > 1.0f) cameraPosition.y = 1.0f;
	} 
	else 
	{
		m_matDevViewMatrix = glm::lookAt(m_vec3DevCamPos, m_vec3DevCamPos + m_vec3DevCamFront, m_vec3DevCamUp);	
		m_mat4CurrentViewMatrix = m_matDevViewMatrix;
		cameraPosition = m_vec3DevCamPos;
	}

	if(m_bPBOFirstFrame)
	{
		m_structPboInfo.pboPtr = nullptr;	
	}
	else if(!m_bPBOFirstFrame)
	{
		TransferDataToCPU();	

	}

	//std::cout << "PBO : " << static_cast<unsigned>(m_pDataSize[8]) << std::endl;
	m_structPboInfo.pboSize = m_nRenderWidth * m_nRenderHeight * 4 * sizeof(unsigned char);
	//m_structPboInfo.pboPtr = new unsigned char[m_structPboInfo.pboSize];
	//m_structPboInfo.pboPtr = m_pDataSize;

	//glm::vec3 vec3TranslationVal = glm::vec3(m_vec4TranslationVal.x / m_vec4TranslationVal.w, m_vec4TranslationVal.y / m_vec4TranslationVal.w, m_vec4TranslationVal.z / m_vec4TranslationVal.w);
	glm::vec3 vec3TranslationVal = glm::vec3(m_vec4TranslationVal.x, m_vec4TranslationVal.y, m_vec4TranslationVal.z);
	//update variables for fiveCell
	fiveCell.update(m_mat4CurrentViewMatrix, cameraPosition, machineLearning, m_vec3ControllerWorldPos[0], m_vec3ControllerWorldPos[1], m_quatController[0], m_quatController[1], m_structPboInfo, vec3TranslationVal);

	//delete[] m_pDataSize;
	delete[] m_structPboInfo.pboPtr;

	//std::cout << shaderData.size() << std::endl;
	// write shaderData to file each frame to see output
	//std::ofstream dataFile;
	//double timeStamp = glfwGetTime();
	//std::string fileName = "dataFile" + std::to_string(timeStamp) + ".txt";
	//dataFile.open(fileName);
	//for(int i = 0; i < shaderData.size(); i++)
	//{
	//	dataFile << i << " -- " << shaderData[i] << "\n";
	//}
	//dataFile.close();

	//for(int i = 0; i < shaderData.size(); i++)
	//{
	//	std::cout << shaderData[i] << std::endl;
	//}
	//shaderData.clear();
}

//-----------------------------------------------------------------------------
// Convert the data texture from multisample to non-multisample so
// glGetTexImage can be used to write contents to PBO
//-----------------------------------------------------------------------------
void Graphics::BlitDataTexture()
{
	// clear error flag
	glGetError();

	glDisable(GL_MULTISAMPLE);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glReadBuffer(buffers[1]);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_gluiStorageFBO);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

   	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glEnable(GL_MULTISAMPLE);
}

//-----------------------------------------------------------------------------
// Main function that renders textures to hmd.
//-----------------------------------------------------------------------------
bool Graphics::BRenderFrame(std::unique_ptr<VR_Manager>& vrm)
{
	float currentFrame = glfwGetTime();
	m_fDeltaTime = currentFrame - m_fLastFrame;

	//update eye independent values for the scene before rendering
	UpdateSceneData(vrm);

	// for now as fast as possible
	if ( !m_bDevMode && vrm->m_pHMD )
	{
		GetControllerEvents(vrm);
		RenderControllerAxes(vrm);
		RenderStereoTargets(vrm);
		BlitDataTexture();
		WriteDataToPBO();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

		m_fLastFrame = currentFrame;
	} else if(m_bDevMode && vrm == nullptr){
		
		DevProcessInput(m_pGLContext);
		RenderStereoTargets(vrm);
		BlitDataTexture();
		WriteDataToPBO();
		RenderCompanionWindow();
		m_fLastFrame = currentFrame;
	} else if(!m_bDevMode && vrm == nullptr){
		std::cout << "ERROR: vrm not assigned : RenderFrame()" << std::endl;
		return true;
	}
		
	if (m_bVblank && m_bGLFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		glfwSwapBuffers(m_pGLContext);
		if(m_bDevMode) glfwSetCursorPosCallback(m_pGLContext, DevMouseCallback);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	if(!m_bDevMode){
		// Spew out the controller and pose count whenever they change.
		if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || vrm->m_iValidPoseCount != m_iValidPoseCount_Last )
		{
			m_iValidPoseCount_Last = vrm->m_iValidPoseCount;
			m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
			
			std::printf( "PoseCount:%d(%s) Controllers:%d\n", vrm->m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
		}

		vrm->UpdateHMDMatrixPose();
		// switch PBOs
		m_bPboIndex = !m_bPboIndex;

		return false;
	} 

	// switch PBOs
	m_bPboIndex = !m_bPboIndex;

	return false;
	//m_uiFrameNumber++;
	//std::cout << *m_pRotationVal << std::endl;
}

//-----------------------------------------------------------------------------
// This function increases the angle for the 3D rotation of the structure
// ----------------------------------------------------------------------------
void Graphics::IncreaseRotationValue(std::unique_ptr<int>& pVal){

	//std::cout << "HEY!" << std::endl;
	int count;
	count = *pVal;
	count++;
	int angle = count % 360;
	*pVal = angle;
}
		
//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void Graphics::RenderControllerAxes(std::unique_ptr<VR_Manager>& vrm)
{
	// Don't attempt to update controllers if input is not available
	if( !vrm->m_pHMD->IsInputAvailable() )
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertCount = 0;
	m_iTrackedControllerCount = 0;

	// this for loop shuold use eHand iterators from VR::Manager	
	for (int i = 0; i <= 1; i++)
	{
		if ( !vrm->m_rHand[i].m_bShowController )
			continue;


		const glm::mat4& mat = vrm->m_rHand[i].m_rmat4Pose;

		glm::vec4 center = mat * glm::vec4( 0, 0, 0, 1 );

		for ( int i = 0; i < 3; ++i )
		{
			glm::vec3 color( 0, 0, 0 );
			glm::vec4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back( center.x );
			vertdataarray.push_back( center.y );
			vertdataarray.push_back( center.z );

			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			vertdataarray.push_back( point.x );
			vertdataarray.push_back( point.y );
			vertdataarray.push_back( point.z );
		
			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			m_uiControllerVertCount += 2;
		}

		glm::vec4 start = mat * glm::vec4( 0, 0, -0.02f, 1 );
		glm::vec4 end = mat * glm::vec4( 0, 0, -39.f, 1 );
		glm::vec3 color( .92f, .92f, .71f );

		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		m_uiControllerVertCount += 2;
	}

	// Setup the VAO the first time through.
	if ( m_unControllerVAO == 0 )
	{
		glGenVertexArrays( 1, &m_unControllerVAO );
		glBindVertexArray( m_unControllerVAO );

		glGenBuffers( 1, &m_glControllerVertBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

		GLuint stride = 2 * 3 * sizeof( float );
		uintptr_t offset = 0;

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof( glm::vec3);
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//std::cout << "CONTROLLERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << std::endl;
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}
}

//-----------------------------------------------------------------------------
// Read data from currently bound read framebuffer to pbo
//-----------------------------------------------------------------------------
void Graphics::WriteDataToPBO()
{
	// clear error flag
	glGetError();

	// Bind PBO for current frame
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[m_bPboIndex]);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glBindTexture(GL_TEXTURE_2D, m_gluiDataTexture2D);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}
	
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);	
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	m_bPBOFirstFrame = false;
}

//-----------------------------------------------------------------------------
// Draws textures to each eye of hmd. 
//-----------------------------------------------------------------------------
void Graphics::RenderStereoTargets(std::unique_ptr<VR_Manager>& vrm)
{
	glEnable(GL_MULTISAMPLE);

	// clear error flag 
	glGetError();

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
 	RenderScene(vr::Eye_Left, vrm);
 	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glDisable(GL_MULTISAMPLE);
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);
	
   	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	if(m_bDebugOpenGL && m_bDevMode)
	{
		GLCheckError();
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	if(!m_bDevMode && vrm != nullptr){

		glEnable(GL_MULTISAMPLE);
		
		// Right Eye
		glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
		glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
		RenderScene(vr::Eye_Right, vrm);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_MULTISAMPLE);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

		glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
}

//-----------------------------------------------------------------------------
// Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void Graphics::RenderScene(vr::Hmd_Eye nEye, std::unique_ptr<VR_Manager>& vrm)
{

	glm::mat4 currentProjMatrix;
	glm::mat4 currentEyeMatrix;
	glm::vec3 cameraFront;
	//glm::vec3 cameraPosition;

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if(!m_bDevMode){
		currentProjMatrix = vrm->GetCurrentProjectionMatrix(nEye);
		//currentViewMatrix = vrm->GetCurrentViewMatrix();
		currentEyeMatrix = vrm->GetCurrentEyeMatrix(nEye);
		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		//cameraPosition = glm::vec3(currentViewMatrix[3][0], currentViewMatrix[3][1], currentViewMatrix[3][2]);

		glm::vec4 m_vFarPlaneDimensions = vrm->GetFarPlaneDimensions(nEye);
		raymarchData.tanFovYOver2 = m_vFarPlaneDimensions.w;
	} 
	else 
	{
		//*** put manual matrices here***//
		currentProjMatrix = m_matDevProjMatrix;  
		//m_matDevViewMatrix = glm::lookAt(m_vec3DevCamPos, m_vec3DevCamPos + m_vec3DevCamFront, m_vec3DevCamUp);	
		//currentViewMatrix = m_matDevViewMatrix;
		currentEyeMatrix = glm::mat4(1.0f);
		cameraFront = m_vec3DevCamFront;
		//cameraPosition = m_vec3DevCamPos;

		double fovYRadians = m_fFov * (PI / 180.0f);
		//raymarchData.tanFovYOver2 = atan2(fovYRadians, 1.0f);		
		raymarchData.tanFovYOver2 = tan(fovYRadians / 2.0f);		
	}

	
	//rapidmix data
	//if(machineLearning.bRecord){
	//	std::vector<glm::vec3> input;
	//	std::vector<double> output;
	//	input.push_back(cameraPosition);
	////********* continue here - need to grab some audio and visual parameters and add to output vector *************//
	//}

	//update stuff for raymarching shader
	raymarchData.aspect = static_cast<float>(m_nRenderWidth) / static_cast<float>(m_nRenderHeight);

	//update variables for fiveCell
	//fiveCell.update(currentViewMatrix, cameraPosition, machineLearning);
	
	// draw controllers before scene	
	if(!m_bDevMode && vrm){
	
		bool bIsInputAvailable = vrm->m_pHMD->IsInputAvailable();

		if (bIsInputAvailable)
		{
			// draw the controller axis lines
			glUseProgram(m_unControllerTransformProgramID);
			glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, &vrm->GetCurrentViewProjectionMatrix(nEye)[0][0]);
			glBindVertexArray(m_unControllerVAO);
			glDrawArrays(GL_LINES, 0, m_uiControllerVertCount);
			glBindVertexArray(0);
		}

		// ----- Render Model rendering -----
		glUseProgram(m_unRenderModelProgramID);

		// this for loop should use eHand iterators from VR_Manager
		for (int i = 0; i <= 1; i++)
		{
			if(!vrm->m_rHand[i].m_bShowController || !vrm->m_rHand[i].m_pRenderModel)
				continue;

			//std::cout << "DRAW CONTROLLER" << std::endl;
			const glm::mat4& matDeviceToTracking = vrm->m_rHand[i].m_rmat4Pose;
			glm::mat4 matMVP = vrm->GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
			glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, &matMVP[0][0]);
			vrm->m_rHand[i].m_pRenderModel->Draw();
			glm::mat4 matModelViewController = vrm->GetCurrentViewMatrix() * matDeviceToTracking;
			m_vec3ControllerWorldPos[i] = glm::vec3(matModelViewController[3][0], matModelViewController[3][1], matModelViewController[3][2]); 	

			// convert controller modelView matrix to quaternion
			m_quatController[i] = glm::quat_cast(matModelViewController);	
		}

		glUseProgram(0);
	}

	//draw fiveCell scene
	fiveCell.draw(currentProjMatrix, m_mat4CurrentViewMatrix, currentEyeMatrix, raymarchData, mengerShaderProg);

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Graphics::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

	glBindVertexArray(m_unCompanionWindowVAO);
	glUseProgram(m_unCompanionWindowProgramID);

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize, GL_UNSIGNED_SHORT, 0);
	// render right eye (second half of index array )
	//glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	// Read pixels to memory to save as png **to do - change to C++14 style**
	if(m_bRecordScreen)
	{
		GLubyte *pixels = (GLubyte*) malloc(4 * m_nCompanionWindowWidth * m_nCompanionWindowHeight);
		if(pixels)
		{
			glReadPixels(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}
	
		//WriteToPNG(pixels);

		free(pixels);
		pixels = nullptr;
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

//----------------------------------------------------------------------
// Writes data to PNG file. 
//----------------------------------------------------------------------
void Graphics::WriteToPNG(GLubyte* &data){

	char* filename = (char*) malloc(32);
	snprintf(filename, sizeof(char) * 32, "stills3/image%04d.png", m_unImageCount);

	//FILE* imageFile;
	//imageFile = fopen(filename, "wb");
	//if(imageFile == NULL){
	//	std::cout << "ERROR: No image file: Graphics::WriteToPNG" << std::endl;
	//}

	// Encode png image
	unsigned error = lodepng_encode32_file(filename, data, m_nCompanionWindowWidth, m_nCompanionWindowHeight);
	if(error){
		std::cout << "ERROR: Image file not encoded by lodepng: Graphics::WriteToPNG" << std::endl;
	}

	m_unImageCount++;
	free(filename);
	filename = nullptr;
}

//----------------------------------------------------------------------
// Deletes render buffers and textures for each eye of the hmd.
// ---------------------------------------------------------------------
void Graphics::CleanUpGL(std::unique_ptr<VR_Manager>& vrm){

	if(m_pGLContext){

		if( m_bDebugOpenGL && !m_bDevMode)
		{
			glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
			glDebugMessageCallback(nullptr, nullptr);
		}

		glDeleteBuffers(1, &m_glSceneVBO);

		if (m_glMainShaderProgramID)
		{
			glDeleteProgram(m_glMainShaderProgramID);
		}
		if ( m_unControllerTransformProgramID )
		{
			glDeleteProgram( m_unControllerTransformProgramID );
		}
		if ( m_unRenderModelProgramID )
		{
			glDeleteProgram( m_unRenderModelProgramID );
		}
		if ( m_unCompanionWindowProgramID )
		{
			glDeleteProgram( m_unCompanionWindowProgramID );
		}	

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		if(!m_bDevMode && vrm){
			glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
			glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
			glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
			glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
			glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );
		}

		if( m_unCompanionWindowVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
		}
		if( m_unSceneVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unSceneVAO );
		}
		if( m_unControllerVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unControllerVAO );
		}

		glfwTerminate();

		fiveCell.exit();
		//delete[] m_structPboInfo.pboPtr;
		//delete[] m_pDataSize;

	}
}

//-------------------------------------------------
// Temporary function to escape the main loop
// ------------------------------------------------
bool Graphics::TempEsc(){

	glfwPollEvents();

	if(GLFW_PRESS == glfwGetKey(m_pGLContext, GLFW_KEY_ESCAPE)){
		glfwSetWindowShouldClose(m_pGLContext, 1);
			return true;
	}

	return false;
}

