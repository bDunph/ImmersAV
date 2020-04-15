#version 410

layout(location = 0) in vec3 position;
 
//uniform mat4 view;
//uniform mat4 proj;
//uniform mat4 eyeMat;
uniform mat4 MVEPMat;
uniform mat4 InvMVEP;
uniform mat4 MVEMat;
uniform mat4 InvMVE;

//uniform float aspect;
//uniform float fovYScale;

//out vec4 eye;
//out vec4 worldDir;
//out vec4 cameraForward;
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


//*********************************************************************************************
	//gl_Position = vec4(position, 1.0);

	//mat4 invEye = inverse(eyeMat);
	//mat4 invView = inverse(view);
	//mat4 invProj = inverse(proj);

	//mat4 viewEye = eyeMat * view;

	////vec4 eyeInv = vec4(-eyeMat[3].xyz, 1.0);
	//vec3 camPos = vec3(viewEye[0][3], viewEye[1][3], viewEye[2][3]);
	//vec4 eyeInv = vec4(camPos, 1.0);
	//eye = invView * invEye * eyeInv;

	////vec2 screenPos = vec2((position.x - eyeMat[3].x) * fovYScale * aspect, position.y * fovYScale);
	//vec2 screenPos = vec2((position.x - viewEye[0][3]) * fovYScale * aspect, position.y * fovYScale);

	//vec4 ndcRay = vec4(screenPos, -1.0, 0.0);
	//worldDir = invView * invEye * ndcRay;

	//vec4 rawcameraForward = vec4(0.0, 0.0, -1.0, 0.0);
	//cameraForward = invView * invEye * rawcameraForward;
}
