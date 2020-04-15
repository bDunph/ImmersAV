#version 410

layout(location = 0) in vec3 position;
 
uniform mat4 view;
uniform mat4 proj;
uniform mat4 eyeMat;

uniform float aspect;
uniform float fovYScale;

out vec4 eye;
out vec4 worldDir;
out vec4 cameraForward;

void main() {
	gl_Position = vec4(position, 1.0);

	mat4 invEye = inverse(eyeMat);
	mat4 invView = inverse(view);
	mat4 invProj = inverse(proj);

	vec4 eyeInv = vec4(-eyeMat[3].xyz, 1.0);
	eye = invView * invEye * eyeInv;

	vec2 screenPos = vec2((position.x - eyeMat[3].x) * fovYScale * aspect, position.y * fovYScale);
	vec4 ndcRay = vec4(screenPos, -1.0, 0.0);
	worldDir = invView * invEye * ndcRay;

	vec4 rawcameraForward = vec4(0.0, 0.0, -1.0, 0.0);
	cameraForward = invView * invEye * rawcameraForward;
}
