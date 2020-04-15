#version 410

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;

//uniform mat4 proj;
//uniform mat4 eyeMat;
//uniform mat4 view;
uniform mat4 MVEPMat;

//in vec4 eye;
//in vec4 worldDir;
//in vec4 cameraForward;
in vec4 nearPos;
in vec4 farPos;

out vec4 fragColor; 

///**
// * Constructive solid geometry intersection operation on SDF-calculated distances.
// */
float intersectSDF(float distA, float distB) {
    return max(distA, distB);
}

///**
// * Constructive solid geometry union operation on SDF-calculated distances.
// */
float unionSDF(float distA, float distB) {
    return min(distA, distB);
}

///**
// * Constructive solid geometry difference operation on SDF-calculated distances.
// */
float differenceSDF(float distA, float distB) {
    return max(distA, -distB);
}

float boxSDF(vec3 p, vec3 size) {
    vec3 d = abs(p) - (size / 2.0);
    
    // Assuming p is inside the cube, how far is it from the surface?
    // Result will be negative or zero.
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    
    // Assuming p is outside the cube, how far is it from the surface?
    // Result will be positive or zero.
    float outsideDistance = length(max(d, 0.0));
    
    return insideDistance + outsideDistance;
}

float box2DSDF(vec2 p, vec2 size) {
    vec2 d = abs(p) - (size / 2.0);
    
    // Assuming p is inside the cube, how far is it from the surface?
    // Result will be negative or zero.
    float insideDistance = min(max(d.x, d.y), 0.0);
    
    // Assuming p is outside the cube, how far is it from the surface?
    // Result will be positive or zero.
    float outsideDistance = length(max(d, 0.0));
    
    return insideDistance + outsideDistance;
}

float crossSDF(vec3 p){
    
    float xArm = box2DSDF(p.xy, vec2(1.0));
    float yArm = box2DSDF(p.yz, vec2(1.0));
    float zArm = box2DSDF(p.zx, vec2(1.0));
                        
    return unionSDF(xArm, unionSDF(yArm, zArm));
}

float sceneSDF(vec3 samplePoint) {    
    float cube = boxSDF(samplePoint, vec3(1.0, 1.0, 1.0));
    float cubeCross = crossSDF(samplePoint * 3.0) / 3.0;    
    cube = differenceSDF(cube, cubeCross);

    float iterativeScalar = 1.0;
    
    for(int i = 0; i <4; i++){
     	
        vec3 a = mod(samplePoint * iterativeScalar, 2.0) - 1.0;
        iterativeScalar *= 3.0;
        vec3 r = abs(1.0 - 4.0 * abs(a));
        float cubeCross = crossSDF(r) / iterativeScalar;    
        cube = differenceSDF(cube, cubeCross);
    }
    
    return cube;
}

///**
// * Return the shortest distance from the eyepoint to the scene surface along
// * the marching direction. If no part of the surface is found between start and end,
// * return end.
// * 
// * eye: the eye point, acting as the origin of the ray
// * marchingDirection: the normalized direction to march in
// * start: the starting distance away from the eye
// * end: the max distance away from the ey to march before giving up
// */
float shortestDistanceToSurface(vec3 eye, vec3 marchingDirection, float start, float end) {
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eye + depth * marchingDirection);
        if (dist < EPSILON) {
		return depth;
        }
        depth += dist;
        if (depth >= end) {
            return end;
        }
    }
    return end;
}

///**
// * Using the gradient of the SDF, estimate the normal on the surface at point p.
// */
vec3 estimateNormal(vec3 p) {
    return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

///**
// * Lighting contribution of a single point light source via Phong illumination.
// * 
// * The vec3 returned is the RGB color of the light's contribution.
// *
// * k_a: Ambient color
// * k_d: Diffuse color
// * k_s: Specular color
// * alpha: Shininess coefficient
// * p: position of point being lit
// * eye: the position of the camera
// * lightPos: the position of the light
// * lightIntensity: color/intensity of the light
// *
// * See https://en.wikipedia.org/wiki/Phong_reflection_model#Description
// */
vec3 phongContribForLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye,
                          vec3 lightPos, vec3 lightIntensity) {
    vec3 N = estimateNormal(p);
    vec3 L = normalize(lightPos - p);
    vec3 V = normalize(eye - p);
    vec3 R = normalize(reflect(-L, N));
    
    float dotLN = dot(L, N);
    float dotRV = dot(R, V);
    
    if (dotLN < 0.0) {
        // Light not visible from this point on the surface
        return vec3(0.0, 0.0, 0.0);
    } 
    
    if (dotRV < 0.0) {
        // Light reflection in opposite direction as viewer, apply only diffuse
        // component
        return lightIntensity * (k_d * dotLN);
    }
    return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

///**
// * Lighting via Phong illumination.
// * 
// * The vec3 returned is the RGB color of that point after lighting is applied.
// * k_a: Ambient color
// * k_d: Diffuse color
// * k_s: Specular color
// * alpha: Shininess coefficient
// * p: position of point being lit
// * eye: the position of the camera
// *
// * See https://en.wikipedia.org/wiki/Phong_reflection_model#Description
// */
vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye) {
    const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
    vec3 color = ambientLight * k_a;
    
    vec3 light1Pos = vec3(4.0,
                          2.0,
                          4.0);
    vec3 light1Intensity = vec3(0.4, 0.4, 0.4);
    
    color += phongContribForLight(k_d, k_s, alpha, p, eye,
                                  light1Pos,
                                  light1Intensity);
    
    vec3 light2Pos = vec3(2.0 * sin(0.37),
                          2.0 * cos(0.37),
                          2.0);
    vec3 light2Intensity = vec3(0.4, 0.4, 0.4);
    
    color += phongContribForLight(k_d, k_s, alpha, p, eye,
                                  light2Pos,
                                  light2Intensity);    
    return color;
}

void main()
{

	//************* code from https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html ************//

	vec3 rayOrigin = nearPos.xyz / nearPos.w;
	vec3 rayEnd = farPos.xyz / farPos.w;
	vec3 rayDir = rayEnd - rayOrigin;
	rayDir = normalize(rayDir);	

    	float dist = shortestDistanceToSurface(rayOrigin, rayDir, MIN_DIST, MAX_DIST);
    
    	if (dist > MAX_DIST - EPSILON) {
        	// Didn't hit anything
        	fragColor = vec4(0.0, 0.0, 0.0, 0.0);
			return;
    	}
    
    	// The closest point on the surface to the eyepoint along the view ray
    	vec3 p = rayOrigin + dist * rayDir;

    	// Use the surface normal as the ambient color of the material
    	vec3 K_a = (estimateNormal(p) + vec3(1.0)) / 2.0;
    	vec3 K_d = K_a;
    	vec3 K_s = vec3(1.0, 1.0, 1.0);
    	float shininess = 10.0;
    
    	vec3 color = phongIllumination(K_a, K_d, K_s, shininess, p, rayOrigin);
    
    	fragColor = vec4(color, 1.0);

	vec4 pClipSpace =  MVEPMat * vec4(p, 1.0);
	vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	float ndcDepth = pNdc.z;
	
	float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	gl_FragDepth = d;

//******************************************************************************************
	//vec3 dir = normalize(worldDir.xyz);
    	//float dist = shortestDistanceToSurface(eye.xyz, dir, MIN_DIST, MAX_DIST);
    
    	//if (dist > MAX_DIST - EPSILON) {
        //	// Didn't hit anything
        //	fragColor = vec4(0.0, 0.0, 0.0, 0.0);
	//		return;
    	//}
    
    	//// The closest point on the surface to the eyepoint along the view ray
    	//vec3 p = eye.xyz + dist * dir;

    	//// Use the surface normal as the ambient color of the material
    	//vec3 K_a = (estimateNormal(p) + vec3(1.0)) / 2.0;
    	//vec3 K_d = K_a;
    	//vec3 K_s = vec3(1.0, 1.0, 1.0);
    	//float shininess = 10.0;
    
    	//vec3 color = phongIllumination(K_a, K_d, K_s, shininess, p, eye.xyz);
    
    	//fragColor = vec4(color, 1.0);

	//vec4 pClipSpace = proj * eyeMat * view * vec4(p, 1.0);
	//vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	//float ndcDepth = pNdc.z;
	//
	//float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	//gl_FragDepth = d;
}
