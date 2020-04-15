#version 410

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 soundModelMat;

out vec3 fragPos_worldSpace;
out vec3 normal_worldSpace;

void main(){

	gl_Position = projMat * viewMat * soundModelMat * vec4(position, 1.0);	

	fragPos_worldSpace = vec3(soundModelMat * vec4(position, 1.0)).xyz;  	
	normal_worldSpace = mat3(transpose(inverse(soundModelMat))) * normal;

}
