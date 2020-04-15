#include "AvrApp.hpp"

#include <iostream>

#ifdef __APPLE__
#define _stricmp strcasecmp
#include <strings.h>
#endif

//--------------------------------------------
AvrApp::AvrApp(int argc, char** argv) : 
	m_pVR(nullptr), 
	m_pGraphics(nullptr), 
	m_pExFlags(nullptr),
//	m_pAudio(nullptr),
	m_bDebugGL(false),
	m_bVSyncBlank(true),
	m_bOpenGLFinishHack(true),
	m_bPrintDebugMsgs(false),
	m_bDevMode(false)  
{

	for( int i = 0; i < argc; i++ )
	{
		if( !_stricmp(argv[i], "-gldebug"))
		{
			m_bDebugGL = true;
		}
		else if( !_stricmp( argv[i], "-novblank" ) )
		{
			m_bVSyncBlank = false;
		}
		else if( !_stricmp( argv[i], "-noglfinishhack" ) )
		{
			m_bOpenGLFinishHack = false;
		}
		else if(!_stricmp(argv[i], "-printdebugmsgs"))
		{
			m_bPrintDebugMsgs = true;
		}
		else if(!_stricmp(argv[i], "-dev"))
		{
			m_bDevMode = true;
		}
	}	

	m_pExFlags = std::make_unique<ExecutionFlags>();

	m_pExFlags->flagDebugOpenGL = m_bDebugGL;
	m_pExFlags->flagVBlank = m_bVSyncBlank;
	m_pExFlags->flagGLFinishHack = m_bOpenGLFinishHack;
	m_pExFlags->flagDPrint = m_bPrintDebugMsgs;
	m_pExFlags->flagDevMode = m_bDevMode;
}

//--------------------------------------------
bool AvrApp::BInitialise()
{
	if(!m_pExFlags->flagDevMode)
	{
		//initialise OpenVR
		m_pVR = std::make_unique<VR_Manager>(m_pExFlags);

		if(!m_pVR->BInit())
		{
			std::cout << "Error: OpenVR system not initialised!" << std::endl;
			return false;
		} 
		else if(!m_pVR->BInitCompositor())
		{
			std::cout << "Error: OpenVR compositor not initialised!" << std::endl;
			return false;
		} 
		else if(!m_pVR->BSetupCameras())
		{ 
			std::cout << "Error: Cameras not set up" << std::endl;
			return false;
		}

		std::cout << "OpenVR initialised" << std::endl;
	}

	//initialise OpenGL
	m_pGraphics = std::make_unique<Graphics>(m_pExFlags);

	if(!m_pGraphics->BInitGL())
	{
		std::cout << "Error: OpenGL context not initialised!" << std::endl;
		return false;
	} 
	else if(!m_pGraphics->BSetupStereoRenderTargets(m_pVR))
	{
			std::cout << "Error: Stereo render targets not set up" << std::endl;
			return false;
	}
	else if(!m_pGraphics->BCreateDefaultShaders())
	{
		std::cout << "Error: Default shaders not set up" << std::endl;
		return false;
	}  
	else if(!m_pGraphics->BSetupCompanionWindow())
	{
		std::cout << "Error: Companion window not set up" << std::endl;
		return false;
	} 

	std::cout << "OpenGL initialised" << std::endl;
	
	//initialise CSound
//	m_pAudio = std::make_unique<CsoundSession>(csdName);
//	if(m_pAudio == NULL){
//		std::cout << "Error: CSound thread not initialised!" << std::endl;
//		return false;
//	}
//
//	std::cout << "CSound initialised" << std::endl;

	return true;	
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void AvrApp::RunMainLoop()
{

	bool bQuit = false;

	while (!bQuit)
	{
		if(!m_pExFlags->flagDevMode){
			bQuit = m_pVR->HandleInput();
		}
		bQuit = m_pGraphics->BRenderFrame(m_pVR);
		
		bQuit = m_pGraphics->TempEsc();
	}
}

//-----------------------------------------
// Clean up each of the modules
// ----------------------------------------
void AvrApp::Exit(){

	if(!m_pExFlags->flagDevMode){
		m_pVR->ExitVR();
	}

	m_pGraphics->CleanUpGL(m_pVR);
}
