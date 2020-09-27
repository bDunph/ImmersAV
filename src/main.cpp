#include "immersAVConfig.h"
#include "ImmersAVApp.hpp"

#include <iostream>
#include <memory>

int main(int argc, char** argv){
	
	//output version number
	std::cout << argv[0] <<  " Version " << immersAV_VERSION_MAJOR << "." << immersAV_VERSION_MINOR << std::endl; 
	
//	if(argc <= 1){
//		std::cout << "Error: need to provide name of csd file" << std::endl;
//		return 1;
//	}

	std::unique_ptr<ImmersAVApp> immersAV = std::make_unique<ImmersAVApp>(argc, argv);
	bool immersAVInit = immersAV->BInitialise();

	if(!immersAVInit){
		std::cout << "Error: ImmersAV failed to initialise!" << std::endl;
		return 1;
	}

	std::cout << "ImmersAV initialised" << std::endl;

	immersAV->RunMainLoop();
	
	immersAV->Exit();

	return 0;
}
