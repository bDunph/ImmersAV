#version 410

layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 texCoordIn;
//layout (location = 1) in vec3 normal;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 quadModelMat;

//out vec3 fragPos_worldSpace;
//out vec3 normal_worldSpace;
out vec2 texCoord;

void main(){
	
	//vec4 projectedTexCoord = projMat * viewMat * quadModelMat * vec4(texCoordIn.x, texCoordIn.y, 0.0, 1.0);
	//texCoord = projectedTexCoord.xy;
	
	vec4 projectedPos = projMat * viewMat * quadModelMat * vec4(position, 1.0);

	texCoord = position.xy;

	gl_Position = projectedPos;
 
	//fragPos_worldSpace = vec3(quadModelMat * vec4(position, 1.0)).xyz;  	
	//vec3 normal = vec3(0.0, 0.0, 1.0);
	//vec4 norm = transpose(inverse(quadModelMat)) * vec4(normal, 1.0);
	//normal_worldSpace = norm.xyz;

}
