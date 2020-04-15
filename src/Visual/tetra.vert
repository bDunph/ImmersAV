#version 410

layout(location = 0) in vec3 position;

uniform mat4 viewEyeProjMat;

void main() {

	gl_Position = viewEyeProjMat * vec4(position * 10.0, 1.0);

	
}  
