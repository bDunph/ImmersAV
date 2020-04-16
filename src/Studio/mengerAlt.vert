#version 410

layout(location = 0) in vec3 position;
 
uniform mat4 MVEPMat;
uniform mat4 InvMVEP;

out vec4 nearPos;
out vec4 farPos;

void main() {
	
	//********* code from https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html *******//

	gl_Position = vec4(position, 1.0);

	//get 2D projection of this vertex in normalised device coordinates
	vec2 ndcPos = gl_Position.xy / gl_Position.w;
	
	//compute rays start and end points in the unit cube defined by ndc's
	nearPos = InvMVEP * vec4(ndcPos, -1.0, 1.0);
	farPos = InvMVEP * vec4(ndcPos, 1.0, 1.0);
}
