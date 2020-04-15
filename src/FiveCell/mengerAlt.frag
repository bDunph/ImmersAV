#version 410

#define DO_FRESNEL 1
#define DO_REFLECTION 1
#define DO_REFRACTION 1

// refractive index of some common materials:
// http://hyperphysics.phy-astr.gsu.edu/hbase/Tables/indrf.html
#define REFRACTIVE_INDEX_OUTSIDE 1.00029
#define REFRACTIVE_INDEX_INSIDE  1.125

#define MAX_RAY_BOUNCES 5
#define OBJECT_ABORB_COLOUR vec3(8.0, 8.0, 3.0)

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;
const float GAMMA = 2.2;
const float REFLECT_AMOUNT = 0.01;

uniform mat4 MVEPMat;
uniform float randSize; 
uniform float rmsModVal;
uniform samplerCube skybox;

in vec4 nearPos;
in vec4 farPos;

out vec4 fragColorOut; 

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
     	
        vec3 a = mod((samplePoint * sin(rmsModVal)) * iterativeScalar, 2.0) - 1.0;
        //vec3 a = mod(samplePoint * iterativeScalar, 2.0) - 1.0;
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
float shortestDistanceToSurface(vec3 eye, vec3 marchingDirection, float start, float end, float uniformScaleValue) {
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        //float dist = sceneSDF((eye + depth * marchingDirection) / (randSize + 1.0)) * (randSize + 1.0);
	vec3 pointPos = eye + depth * marchingDirection;
	vec3 translatedPoint = pointPos + vec3(0.0, 0.0, 0.0);
        float dist = sceneSDF(translatedPoint / uniformScaleValue) * uniformScaleValue;
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
vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye, float uniformScaleValue) {

	//vec3 scaleVec =  vec3(uniformScaleValue, uniformScaleValue, uniformScaleValue);

    	const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
	vec3 color = ambientLight * k_a;
    
	vec3 light1Pos = vec3(-4.0, 2.0, -4.0);
	//light1Pos += scaleVec;
	vec3 light1Intensity = vec3(0.8, 0.8, 0.8);
    
	color += phongContribForLight(k_d, k_s, alpha, p, eye, light1Pos, light1Intensity);
    
	//vec3 light2Pos = vec3(4.0, 2.0, 4.0);
	//light2Pos += scaleVec;
	//vec3 light2Intensity = vec3(0.9, 0.9, 0.9);
    
	//color += phongContribForLight(k_d, k_s, alpha, p, eye, light2Pos, light2Intensity);    

	//vec3 light3Pos = vec3(-5.0, 3.0, -4.0);

    	//vec3 light3Intensity = vec3(0.4, 0.4, 0.4);
    
    	//color += phongContribForLight(k_d, k_s, alpha, p, eye,
        //                          light3Pos,
        //                          light3Intensity);    

	return color;
}

//===========================================================
// To calculate the fresnel reflection amout - taken from demofox's shader - 
//	https://www.shadertoy.com/view/4tyXDR
//============================================================
float FresnelReflectAmount (float refIndOut, float refIndIn, vec3 normal, vec3 incident)
{
    #if DO_FRESNEL
        // Schlick aproximation
        float r0 = (refIndOut-refIndIn) / (refIndOut+refIndIn);
        r0 *= r0;
        float cosX = -dot(normal, incident);
        if (refIndOut > refIndIn)
        {
            float n = refIndOut/refIndIn;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return 1.0;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;

        // adjust reflect multiplier for object reflectivity
        ret = (REFLECT_AMOUNT + (1.0-REFLECT_AMOUNT) * ret);
        return ret;
    #else
    	return REFLECT_AMOUNT;
    #endif
}

//============================================================


//============================================================
// Returns the colour reflected or refracted from ray cast from
// surface of an object
//============================================================
vec3 GetColourFromScene(in vec3 rayPosition, in vec3 rayDirection){

	//only factoring in skybox reflections at the moment
	//need to add ground plane

	return texture(skybox, rayDirection).rgb;


}
//============================================================

//============================================================
// Simulates total internal reflection and Beer's Law 
// code from  https://www.shadertoy.com/view/4tyXDR
//============================================================
vec3 GetObjectInternalRayColour(in vec3 rayPos, in vec3 rayDirection){

	float multiplier = 1.0;
	vec3 returnVal = vec3(0.0);
	float absorbDist = 0.0;
	for(int i = 0; i < MAX_RAY_BOUNCES; ++i){

		//try to intersect the object
		float distance = shortestDistanceToSurface(rayPos, rayDirection, MIN_DIST, MAX_DIST, 1.0);
		vec3 intersectPoint = rayPos + rayDirection * distance;
		vec3 inNorm = estimateNormal(intersectPoint); 		

		if(distance < 0.0) return returnVal;

		//move ray position to intersection point
		rayPos = rayPos + rayDirection * distance;
		
		//calculate Beer's Law absorption
		absorbDist += distance;
		vec3 absorbVal = exp(-OBJECT_ABORB_COLOUR * absorbDist);

		//calculate how much to reflect or transmit
		float reflectMult = FresnelReflectAmount(REFRACTIVE_INDEX_INSIDE, REFRACTIVE_INDEX_OUTSIDE, inNorm, rayDirection);  
		float refractMult = 1.0 - reflectMult;
		
		//add in refraction outside of the object
		vec3 refractDirection = refract(rayDirection, inNorm, REFRACTIVE_INDEX_INSIDE / REFRACTIVE_INDEX_OUTSIDE);
		returnVal = GetColourFromScene(rayPos + refractDirection * 0.001, refractDirection) * refractMult * multiplier * absorbVal;

		//add specular highlight based on refracted ray direction
		returnVal += phongIllumination(vec3(0.1), vec3(0.1), vec3(1.0), 1.0, intersectPoint, rayPos,  1.0) * refractMult * multiplier * absorbVal;
		
		//follow ray down internal reflection path
		rayDirection = reflect(rayDirection, inNorm);
		
		//move the ray down the ray path a bit
		rayPos += rayDirection * 0.001;
		
		//recursively add reflectMult amout to consecutive bounces
		multiplier *= reflectMult; 
	}
		
	return returnVal;
}
//============================================================

void main()
{

	//************* code from https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html ************//

	vec3 rayOrigin = nearPos.xyz / nearPos.w;
	vec3 rayEnd = farPos.xyz / farPos.w;
	vec3 rayDir = rayEnd - rayOrigin;
	rayDir = normalize(rayDir);	

	float uniformScaleVal = 1.0;
    	float dist = shortestDistanceToSurface(rayOrigin, rayDir, MIN_DIST, MAX_DIST, uniformScaleVal);
    
    	if (dist > MAX_DIST - EPSILON) {
        	// Didn't hit anything
        	fragColorOut = vec4(0.0, 0.0, 0.0, 0.0);
			return;
    	}
    
    	// The closest point on the surface to the eyepoint along the view ray
    	vec3 p = rayOrigin + dist * rayDir;

	vec3 incidentNormal = estimateNormal(p);

	vec3 color = vec3(0.0);
	vec3 returnVal = vec3(0.0);

    	// Use the surface normal as the ambient color of the material
    	vec3 K_a_orig = (incidentNormal + vec3(1.0)) / 2.0;
    	vec3 K_a_mine = vec3(0.583, 0.095, 0.05);
	vec3 K_a = K_a_orig * K_a_mine;
	//vec3 K_a = vec3(0.1);	
    	vec3 K_d = K_a;
    	vec3 K_s = vec3(1.0, 1.0, 1.0);
    	float shininess = 1.0;
    
    	color += phongIllumination(K_a, K_d, K_s, shininess, p, rayOrigin, uniformScaleVal);

	//following demofox blog and shadertoy for reflection etc. https://www.shadertoy.com/view/4tyXDR and https://blog.demofox.org/2017/01/09/raytracing-reflection-refraction-fresnel-total-internal-reflection-and-beers-law/

	//move ray to intersection with cube
	rayOrigin += rayDir * p;		

	//calculate how much to reflect or transmit
	float reflectionScaleVal = FresnelReflectAmount(REFRACTIVE_INDEX_OUTSIDE, REFRACTIVE_INDEX_INSIDE, incidentNormal, rayDir);	
	float refractScaleVal = 1.0 - reflectionScaleVal;

	//get reflection colour
#if DO_REFLECTION
	vec3 reflectDirection = reflect(rayDir, incidentNormal);
	returnVal += GetColourFromScene(rayOrigin + reflectDirection * 0.001, reflectDirection) * reflectionScaleVal;	
#endif

	//get refraction colour
#if DO_REFRACTION
	vec3 refractDirection = refract(rayDir, incidentNormal, REFRACTIVE_INDEX_OUTSIDE / REFRACTIVE_INDEX_INSIDE);
	returnVal += GetObjectInternalRayColour(rayOrigin + refractDirection * 0.001, refractDirection) * refractScaleVal;		
#endif
	//gamma correction
	vec3 fragColor = pow(color + returnVal, vec3(1.0 / GAMMA));
    	fragColorOut = vec4(fragColor, 1.0);

//-----------------------------------------------------------------------------
// To calculate depth for use with rasterized material
//-----------------------------------------------------------------------------
	vec4 pClipSpace =  MVEPMat * vec4(p, 1.0);
	vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	float ndcDepth = pNdc.z;
	
	float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	gl_FragDepth = d;
}
