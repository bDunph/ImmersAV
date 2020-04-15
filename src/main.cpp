#include "avrConfig.h"
#include "AvrApp.hpp"

#include <iostream>
#include <memory>

int main(int argc, char** argv){
	
	//output version number
	std::cout << argv[0] <<  " Version " << avr_VERSION_MAJOR << "." << avr_VERSION_MINOR << std::endl; 
	
//	if(argc <= 1){
//		std::cout << "Error: need to provide name of csd file" << std::endl;
//		return 1;
//	}

	std::unique_ptr<AvrApp> avr = std::make_unique<AvrApp>(argc, argv);
	bool avrInit = avr->BInitialise();

	if(!avrInit){
		std::cout << "Error: Avr failed to initialise!" << std::endl;
		return 1;
	}

	std::cout << "Avr initialised" << std::endl;

	avr->RunMainLoop();
	
	avr->Exit();

	return 0;
}
