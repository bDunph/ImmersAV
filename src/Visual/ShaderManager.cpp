#include <cstdio>
#include <cstdlib>

#ifdef __APPLE__
#include <GL/glew.h>
#elif _WIN32
#include "GL/glew.h"
#endif

#include "ShaderManager.hpp"
#include "Log.hpp"

#include <iostream>
#include <fstream>

bool load_shader(const char* filename, std::string &string){
	//read shaders from file
	//FILE* f = fopen(filename, "r");
	std::ifstream file(filename);
	std::string text;
	std::string textToString;

	while(std::getline(file, text)){
		textToString += text;
		textToString.push_back('\n');
	}

	//std::cout << textToString << std::endl;
	
	string = textToString;

	return true;
	//if(!f){
	//	fprintf(stderr, "ERROR: %s not opened", filename);
	//	return false;
	//}

	////Determine file size
	//fseek(f, 0, SEEK_END);
	//size_t size = ftell(f);

	////calloc() initialises memory to zero
	//string = (char*)calloc(sizeof(char), size + 1);

	//rewind(f);
	////fread((void*)string, sizeof(char), size, f);
	//if(fgets(string, size + 1, f) != NULL){
	//	printf("%s\n", string);
	//	return true;
	//} 
	//
	//return false;

	
}


std::string readFile(const char* filePath){

	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if(!fileStream.is_open()){

		std::cerr << "Could not read file" << filePath << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while(!fileStream.eof()){

		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

bool shader_compile_check(GLuint shader){
	int params = -1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	if(GL_TRUE != params){
		fprintf(stderr, "ERROR: GL shader %i did not compile\n", shader);
		print_shader_info_log(shader);
		return false;
	}
	return true;
}

bool shader_link_check(GLuint program){
	int params = -1;
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	if(GL_TRUE != params){
		fprintf(stderr, "ERROR: could not link shader program GL index %u\n", program);
		return false;
	}
	return true;
}

bool is_valid(GLuint program){
	glValidateProgram(program);
	int params = -1; 
	glGetProgramiv(program, GL_VALIDATE_STATUS, &params);
	printf("program %i GL_VALIDATE_STATUS = %i\n", program, params);
	if(GL_TRUE != params){
		print_program_info_log(program);
		return false;
	}
	return true;
}
