#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

class Skybox {

public:

	bool setup();
	void draw(glm::mat4 projMatSkybox, glm::mat4 viewEyeMatSkybox, GLuint skyboxProg);

private:
	GLuint skyboxShaderProg;
	GLuint skyboxVAO;	
	GLuint skyboxTexID;
	GLuint skyboxIndexBuffer;

	GLint skybox_projMatLoc;
	GLint skybox_viewMatLoc;
};

#endif
