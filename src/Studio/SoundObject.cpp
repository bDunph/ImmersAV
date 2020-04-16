#include "SoundObject.hpp"
#include "ShaderManager.hpp"

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#elif _WIN32 
#include "glfw3.h"
#endif

#include <iostream>
#include <vector>

bool SoundObject::setup(GLuint soundObjProg){

		//Sound source vertices
	float soundVerts [24] = {
		//top left front	
		-1.0f, 1.0f, 1.0f,
		//bottom left front
		-1.0f, -1.0f, 1.0f,
		//bottom right front
		1.0f, -1.0f, 1.0f,
		//top right front
		1.0f, 1.0f, 1.0f,
		//top left back
		-1.0f, 1.0f, -1.0f,
		//bottom left back
		-1.0f, -1.0f, -1.0f,
		//bottom right back
		1.0f, -1.0f, -1.0f,
		//top right back
		1.0f, 1.0f, -1.0f
	};

	unsigned int soundIndices [36] = {
		//front face
		0, 1, 2,
		0, 2, 3,
		//right face
		3, 2, 6,
		3, 6, 7,
		//back face
		7, 6, 5,
		7, 5, 4,
		//left face
		4, 5, 1,
		4, 1, 0,
		//bottom face
		1, 5, 6,
		1, 6, 2,
		//top face
		4, 0, 3,
		4, 3, 7
	};

	float soundObjNormals [24] = {
		//top front left
		-1.0f, 1.0f, 1.0f,
		//bottom front left
		-1.0f, -1.0f, 1.0f,
		//bottom front right
		1.0f, -1.0f, 1.0f,
		//top front right
		1.0f, 1.0f, 1.0f,
		//top left back
		-1.0f, 1.0f, -1.0f,
		//bottom left back
		-1.0f, -1.0f, -1.0f,
		//bottom right back
		1.0f, -1.0f, -1.0f,
		//top right back
		1.0f, 1.0f, -1.0f
	};	

	//Set up soundobj buffers
	glGenVertexArrays(1, &soundVAO);
	glBindVertexArray(soundVAO);

	GLuint soundVBO;
	glGenBuffers(1, &soundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, soundVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), soundVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint soundObjNormalVBO;
	glGenBuffers(1, &soundObjNormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, soundObjNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), soundObjNormals, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &soundObjIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soundObjIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), soundIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//uniform setup
	soundObj_projMatLoc = glGetUniformLocation(soundObjProg, "projMat");
	soundObj_viewMatLoc = glGetUniformLocation(soundObjProg, "viewMat");
	soundObj_modelMatLoc = glGetUniformLocation(soundObjProg, "soundModelMat");

	soundObj_lightPosLoc = glGetUniformLocation(soundObjProg, "lightPos");
	soundObj_light2PosLoc = glGetUniformLocation(soundObjProg, "light2Pos");

	soundObj_cameraPosLoc = glGetUniformLocation(soundObjProg, "camPos");
	
	soundObj_scaleValLoc = glGetUniformLocation(soundObjProg, "scale");
	
	//only use during development as computationally expensive
	bool validProgram = is_valid(soundObjProg);
	if(!validProgram){
		fprintf(stderr, "ERROR: soundObjProg not valid\n");
		return 1;
	}

	glBindVertexArray(0);
	
	identityModelMat = glm::mat4(1.0);
	
	return true;
}

//void SoundObject::scale(float val){
//
//	glm::vec3 scaleVec = glm::vec3(0.1f + (val * 100.0f), 0.1f + (val * 100.0f), 0.1f + (val * 100.0f));	
//	scaleMat = glm::scale(identityModelMat, scaleVec);
//}

void SoundObject::update(glm::vec3 translationVal, float scaleVal){

	float scaleFull = 0.5f;
	float scaleCalc = scaleVal * scaleFull;
	float scaleBase = 0.1f;
	glm::vec3 scaleVec = glm::vec3(scaleCalc + scaleBase);
	glm::mat4 scaleMat = glm::scale(identityModelMat, scaleVec);
	float soundModelRotAngle = glfwGetTime() * 0.2f;
	glm::mat4 rotateSoundModel = glm::rotate(identityModelMat, soundModelRotAngle, glm::vec3(0, 1, 0));;
	//glm::vec3 finalTranslation = translationVal + glm::vec3(0.0, 2.0, 0.0);
	glm::mat4 translateMat = glm::translate(identityModelMat, translationVal);
	soundModelMatrix = translateMat * rotateSoundModel * scaleMat;
}

void SoundObject::draw(glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 lightPosition, glm::vec3 light2Position, glm::vec3 cameraPosition, GLuint soundObjProg){

	glEnable(GL_CULL_FACE);
	glBindVertexArray(soundVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soundObjIndexBuffer);
	glUseProgram(soundObjProg);

	glUniformMatrix4fv(soundObj_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
	glUniformMatrix4fv(soundObj_viewMatLoc, 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv(soundObj_modelMatLoc, 1, GL_FALSE, &soundModelMatrix[0][0]);
	glUniform3f(soundObj_lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(soundObj_light2PosLoc, light2Position.x, light2Position.y, light2Position.z);
	glUniform3f(soundObj_cameraPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glDrawElements(GL_TRIANGLES, 36 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
