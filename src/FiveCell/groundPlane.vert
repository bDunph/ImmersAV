#version 410

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 groundModelMat;

out vec3 fragPos_worldSpace;
out vec3 normal_worldSpace;
out vec2 texCoordOut;

void main(){
	
	texCoordOut = texCoord;

	gl_Position = projMat * viewMat * groundModelMat * position;
 
	fragPos_worldSpace = vec3(groundModelMat * position).xyz;  	
	vec3 normal = vec3(0.0, 1.0, 0.0);
	normal_worldSpace = mat3(transpose(inverse(groundModelMat))) * normal;

}
