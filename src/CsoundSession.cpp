#include "CsoundSession.hpp"
#include <iostream>


void CsoundSession::StartThread(){
	if(Compile((char *)m_csd.c_str()) == 0){
		m_pt = new CsoundPerformanceThread(this);	
	}
};

//------------------------------------------------------------
void CsoundSession::PlayScore(){
	if(m_pt->GetStatus() == 0) m_pt->Play();

};

void CsoundSession::ResetSession(std::string const &csdFileName){
	if(!csdFileName.empty()) m_csd = csdFileName;

	if(!m_csd.empty()){
		StopPerformance();
		StartThread();
	}
};

void CsoundSession::StopPerformance(){
	if(m_pt){
		if(m_pt->GetStatus() == 0) m_pt->Stop();
		m_pt->Join();
		m_pt = NULL;
	}
	Reset();
};

void CsoundSession::AudioLoop(){
	std::string s;
	bool loop = true;
	while(loop){
		std::cout << std::endl << "l)oad csd; e)vent; r)ewind; t)oggle pause; s)top; p)lay; q)uit: ";
		char c = std::cin.get();
		switch(c) {
			case 'l':
				std::cout << "Enter the name of csd file:";
				std::cin >> s;
				ResetSession(s);
				break;

			case 'e':
				std::cout << "Enter a score event:";
				std::cin.ignore(1000, '\n');
				std::getline(std::cin, s);
				m_pt->InputMessage(s.c_str());
				break;

			case 'r':
				RewindScore();
				break;

			case 't':
				if(m_pt) m_pt->TogglePause();
				break;

			case 's':
				StopPerformance();
				break;

			case 'p':
				ResetSession("");
				break;

			case 'q':
				if(m_pt){
					m_pt->Stop();
					m_pt->Join();
				}
				loop = false;
				break;

		}
		std::cout << std::endl;
	}
};

