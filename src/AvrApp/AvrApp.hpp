#ifndef AVRAPP_HPP
#define AVRAPP_HPP

#include "VR_Manager.hpp"
#include "Graphics.hpp"
//#include "CsoundSession.hpp"
#include "SystemInfo.hpp"

#include <string>
#include <memory>

class AvrApp {

public:
	AvrApp(int argc, char** argv);
	bool BInitialise();
	void Exit();
	void RunMainLoop();
	
private:
	
	std::unique_ptr<VR_Manager> m_pVR;
	std::unique_ptr<Graphics> m_pGraphics;
	//std::unique_ptr<CsoundSession> m_pAudio;
	std::unique_ptr<ExecutionFlags> m_pExFlags;

	bool m_bDebugGL;
	bool m_bVSyncBlank;
	bool m_bOpenGLFinishHack;
	bool m_bPrintDebugMsgs;
	bool m_bDevMode;
};
#endif
