//***********************************************************************************************
// Sound Object
//***********************************************************************************************

#ifndef SOUNDOBJECT_HPP
#define SOUNDOBJECT_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SoundObject {

public:
	bool setup(GLuint soundObjProg);
	void update(glm::vec3 translationVal, float scaleVal);
	void draw(glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 lightPosition, glm::vec3 light2Position, glm::vec3 cameraPosition, GLuint soundObjProg);
private:

	GLuint soundVAO;
	GLuint soundObjIndexBuffer;
	//GLuint soundObjShaderProg;

	GLint soundObj_projMatLoc;
	GLint soundObj_viewMatLoc;
	GLint soundObj_modelMatLoc;
	GLint soundObj_lightPosLoc;
	GLint soundObj_light2PosLoc;
	GLint soundObj_cameraPosLoc;
	GLint soundObj_scaleValLoc;

	glm::mat4 identityModelMat;
	//glm::mat4 scaleMat;
	glm::mat4 soundModelMatrix;
	
};
#endif
