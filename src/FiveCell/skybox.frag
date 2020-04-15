#version 410

const float GAMMA = 2.2;

uniform samplerCube skybox;

in vec3 texCoords;
out vec4 fragColour;

void main(){

	//vec4 colPreGamma = texture(skybox, texCoords);
	//vec3 colPostGamma = pow(colPreGamma.rgb, vec3(1.0 / GAMMA));	
	//fragColour = vec4(colPostGamma, 1.0);
	fragColour = texture(skybox, texCoords);
}
