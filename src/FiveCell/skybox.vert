#version 410

layout (location = 0) in vec3 position;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

out vec3 texCoords;

void main(){

	vec4 projectedPos = projMat * viewMat * modelMat * vec4(position, 1.0);
	texCoords = position;
	//optimisation: setting vert z value to w so depth will always be 1.0
	//therefore making sure skybox will always be behind objects
	gl_Position = projectedPos.xyww;
}
