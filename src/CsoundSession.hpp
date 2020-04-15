#ifndef CSOUNDSESSION_H
#define CSOUNDSESSION_H

#include <string>

#ifdef __APPLE__
#include <csound.hpp>
#elif _WIN32
#include "csound/csound.hpp"
#endif

#include "csPerfThread.hpp"

class CsoundSession : public Csound{

	std::string m_csd;
	CsoundPerformanceThread *m_pt;

public:

	CsoundSession(std::string const &csdFileName) : Csound() {
		m_pt = NULL;
		m_csd = "";
		if(!csdFileName.empty()){
			m_csd = csdFileName;
		}
	};
	void StartThread();
	void PlayScore();
	void ResetSession(std::string const &csdFileName);
	void StopPerformance();
	void AudioLoop();

};

#endif
