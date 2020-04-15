#version 410  
// raymarch basic setup adapted from dila's tutorial
// https://www.youtube.com/watch?v=yxNnRSefK94

#define Iterations 16 
#define MAX_ITERATIONS 100
#define Scale 2.0
#define Offset 2.0
#define NUM_NOISE_OCTAVES 2
#define SUN_DIR vec3(0.5, 0.8, 0.0)
#define EPSILON 0.01
#define NUM_FFT_BINS 512
#define PLANE_NORMAL vec4(0.0, 1.0, 0.0, 0.0)
#define SPHERE_RAD 10.0
#define FACTOR 5.0

uniform mat4 MVEPMat;
uniform float specCentVal;
uniform float lowFreqVal;
uniform float fftAmpBins[NUM_FFT_BINS];
uniform float timeVal;
uniform float sineControlVal;

in vec4 nearPos;
in vec4 farPos;
//in vec2 texCoordsOut;

layout(location = 0) out vec4 fragColor; 
layout(location = 1) out vec4 orbitOut;

int index;
vec4 orbit;

// hash, noise and fbm implementations from morgan3d
// https://www.shadertoy.com/view/4dS3Wd
// By Morgan McGuire @morgan3d, http://graphicscodex.com
// Reuse permitted under the BSD license.

float hash(float p) { p = fract(p * 0.011); p *= p + 7.5; p *= p + p; return fract(p); }

float noise(vec3 x) 
{
    const vec3 step = vec3(110, 241, 171);

    vec3 i = floor(x);
    vec3 f = fract(x);
 
    // For performance, compute the base input to a 1D hash from the integer part of the argument and the 
    // incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
               mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}

float fbm(vec3 x) 
{
	float v = 0.0;
	float a = 0.5;
	vec3 shift = vec3(100);
	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
		v += a * noise(x);
		x = x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

// function from http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

//----------------------------------------------------------------------------------------
// Sphere SDF from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//----------------------------------------------------------------------------------------
float sphereSDF(vec3 p, float radius)
{
	return abs(length(p) - radius);
}

//----------------------------------------------------------------------------------------
// Ground plane SDF from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//----------------------------------------------------------------------------------------

float planeSDF(vec3 p, vec4 normal)
{
	int n = 0;
	while(n < Iterations)
    {
	if(p.x + p.y < 0.0) p.xy = -p.yx; // fold 1
        if(p.x + p.z < 0.0) p.xz = -p.zx; // fold 2
        if(p.y + p.z < 0.0) p.zy = -p.yz; // fold 3

        p = p * Scale - Offset * (Scale - 1.0);
        
        if(length(p) > float(MAX_ITERATIONS)) break;
        
        n++;
    }

	return dot(p, normal.xyz) + normal.w;
}

float kifSDF(vec3 p)
{
	mat3 rot = rotationMatrix(vec3(0.5, 1.0, 0.0), 45.0 + (timeVal * 0.1));
    
 	// sierpinski fractal from http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-iii-folding-space/
    
    orbit = vec4(1000.0);
    
    int n = 0;
    while(n < Iterations)
    {
    
        p = rot * p;
        
        if(p.x + p.y < 0.0) p.xy = -p.yx; // fold 1
        if(p.x + p.z < 0.0) p.xz = -p.zx; // fold 2
        if(p.y + p.z < 0.0) p.zy = -p.yz; // fold 3
        
        p = rot * p;
        
        p = p * Scale - Offset * (Scale - 1.0);
        
        float orbPoint = dot(p, p);
	//int ind = int(floor(mod(timeVal, NUM_FFT_BINS)));
        orbit = min(orbit, vec4(abs(p), orbPoint));
	//orbit *= 1.0 + (lowFreqVal * (gl_FragCoord.x / gl_FragCoord.y));
        
        if(length(p) > float(MAX_ITERATIONS)) break;
        
        n++;
    }
    
    return abs(length(p) * pow(Scale, -float(n)));

}


float recVal = 0.0;

float DE(vec3 p)
{
	float rad = SPHERE_RAD + recVal;

	float kifDist = kifSDF(p);
	float planeDist = planeSDF(p, PLANE_NORMAL);
	float sphereDist = sphereSDF(p, rad);

	if(length(p) > rad) 
	{
		recVal += FACTOR;
		rad += recVal;
		sphereDist = sphereSDF(p, rad);
	}	

	return min(kifDist, min(sphereDist, planeDist));
}



float march(vec3 o, vec3 r)
{
 	float t = 0.0;
    	int ind = 0;
    	for(int i = 0; i < MAX_ITERATIONS; ++i)
    	{
    	 	vec3 p = o + r * t;
    	    	//float d = DE(p * 1.0 + lowFreqVal);

    	    	// twisting deformation
    	    	//float k = 0.01 * lowFreqVal + mod(timeVal, 5.0);
    	    	//float c = cos(k * p.y);
    	    	//float s = sin(k * p.y);
    	    	//mat2 m = mat2(c, -s, s, c);
    	    	//p = vec3(m * p.xz, p.y);

    	    	// sine displacement
    	    	//float factor = fftAmpBins[int(floor(mod(timeVal, NUM_FFT_BINS)))] * 10.0;
    	    	float factor = sin(specCentVal * lowFreqVal);// mod(timeVal, 360.0); 
    	    	float disp = sin(factor * p.x) * sin(factor * p.y) * sin(factor * p.z);
    	    	//disp *= mod(timeVal, 5.0) * lowFreqVal;
    	    	//disp *= sineControlVal * lowFreqVal;
    	    	
    	    	float d = DE(p);

    	    	//vec3 scalingFactor = vec3(5.0, 0.0, 5.0);
    	    	//float noisy = DE(mod(p, scalingFactor + fbm(p * lowFreqVal) - 0.5 * scalingFactor));
    	    	if(d < EPSILON) break;
    	    	t += (d + (disp * 0.2)) * 0.5;
    	    	ind++;
    	}
    	
    	index = ind;
    	return t;
}

// finite difference normal from 
// http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-ii-lighting-and-coloring/
vec3 norm(vec3 pos, vec3 dir)
{
	return normalize(vec3(	DE(pos + vec3(EPSILON, 0.0, 0.0)) - DE(pos - vec3(EPSILON, 0.0, 0.0)),
                			DE(pos + vec3(0.0, EPSILON, 0.0)) - DE(pos - vec3(0.0, EPSILON, 0.0)),
                			DE(pos + vec3(0.0, 0.0, EPSILON)) - DE(pos - vec3(0.0, 0.0, EPSILON))));
}



// ambient occlusion implementation from 
// http://www.pouet.net/topic.php?which=7931&page=1&x=3&y=14
float ao(vec3 p, vec3 n, float d, float i) 
{
	float o;
	for (o=1.;i>0.;i--) {
		o-=(i*d-abs(DE(p+n*i*d)))/pow(2.,i);
	}
	return o;
}

//----------------------------------------------------------------------------------------
// Fog based on https://www.iquilezles.org/www/articles/fog/fog.htm
//----------------------------------------------------------------------------------------
vec3 fog(in vec3 col, in float dist, in vec3 rayDir, in vec3 lightDir)
{

	//float fogAmount = 1.0 - exp(-dist * 0.2);
	vec3 normLightDir = normalize(-lightDir);
	float expDistTerm = exp(dot(rayDir, normLightDir));
	float lightAmount = max(expDistTerm * 0.6, 0.0);
	vec3 fogColour = mix(vec3(0.5, 0.6, 0.7), vec3(1.0, 0.9, 0.7), pow(lightAmount, 10.0));
	//fogColour *= fftVec;

	vec3 bExt = vec3(0.05, 0.03, 0.09);
	vec3 bIns = vec3(0.12, 0.05, 0.05);

	vec3 extCol = vec3(exp(-dist * bExt.x), exp(-dist * bExt.y), exp(-dist * bExt.z));
	vec3 insCol = vec3(exp(-dist * bIns.x), exp(-dist * bIns.y), exp(-dist * bIns.z));

	//float extCol = exp(-dist * 0.02);
	//float insCol = exp(-dist * 0.04);

	//return col += vec3(col * (vec3(1.0) - extCol) + fogColour * (vec3(1.0) - insCol));
	return vec3(col * (vec3(1.0) - extCol) + fogColour * (vec3(1.0) - insCol));
	//return mix(col, fogColour, fogAmount);
}

void main()
{

	//************* ray setup code from 
	//https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html*/
	
	
	//******* Perform raymarch *********************//
	vec3 rayOrigin = nearPos.xyz / nearPos.w;
	vec3 rayEnd = farPos.xyz / farPos.w;
	vec3 rayDir = rayEnd - rayOrigin;
	rayDir = normalize(rayDir);	
	
	// raymarch the point
	float dist = march(rayOrigin, rayDir);
	
	// map audio movement 
	//float pixToBin = mod((1.0 + gl_FragCoord.x * 1.0 + gl_FragCoord.y), NUM_FFT_BINS);
	//float pixToBin = mod(int(floor(fbm(gl_FragCoord.xyz))), NUM_FFT_BINS);
	//int fftIndex = int(floor(pixToBin));
    
	//vec3 fftVec = vec3(float(fftIndex + floor(timeVal)) * timeVal, float(fftIndex + floor(timeVal)) * timeVal, float(fftIndex + floor(timeVal)) * timeVal);
	//float noiseCalc = fbm(fftVec);

	vec3 pos = rayOrigin + dist * rayDir;// + (noiseCalc * 0.01);

		    
	// material colour
	//float specMappedVal = (specCentVal - 20.0) / (10000.0 - 20.0) * (1.0 - 0.0) + 0.0;

	vec3 colour = vec3(0.0);
	vec3 totMatCol = vec3(0.0);

	if(index < MAX_ITERATIONS)
	{
		// colouring and shading
		vec3 norm = norm(pos, rayDir);

		float sq = float(Iterations) * float(Iterations);
		float smootherVal = float(index) + log(log(sq)) / log(Scale) - log(log(dot(pos, pos))) / log(Scale);
		vec3 matCol1 = vec3(pow(0.785, log(smootherVal)), pow(0.38, log(smootherVal)), pow(0.08, log(smootherVal)));
		vec3 matCol2 = vec3(pow(0.15, 1.0 / log(smootherVal)), pow(0.45, 1.0 / log(smootherVal)), pow(0.14, 1.0 / log(smootherVal)));
		totMatCol = mix(matCol1, matCol2, clamp(6.0*orbit.x, 0.0, 1.0));
		//totMatCol = mix(totMatCol, matCol1, pow(clamp(1.0 - 2.0 * orbit.z, 0.0, 1.0), 8.0 + (specMappedVal * fbm(gl_FragCoord.xyz))));

		// lighting
		float ao = ao(pos, norm, 0.5, 5.0);
		float sun = clamp(dot(norm, SUN_DIR), 0.0, 1.0);
		float sky = clamp(0.5 + 0.5 * norm.y, 0.0, 1.0);
		float ind = clamp(dot(norm, normalize(SUN_DIR * vec3(-1.0, 0.0, -1.0))), 0.0, 1.0);
		    
		vec3 lightRig = sun * vec3(1.64, 1.27, 0.99);
		lightRig += sky * vec3(0.16, 0.2, 0.28) * ao;
		lightRig += ind * vec3(0.4, 0.28, 0.2) * ao;
		    
		colour = totMatCol * lightRig;
	}
	else
	{
		colour = vec3(0.16, 0.2, 0.28);
	}
	
	
	    
	//float fog = 1.0 / (1.0 + dist * dist * 0.5);
	    
	//colour = pow(colour, vec3(fog));
	//colour *= fog;
	    
	colour = fog(colour, dist, rayDir, SUN_DIR);

	// gamma corr
	colour = pow(colour, vec3(1.0/2.2));
	
	// Output to screen
	fragColor = vec4(colour,1.0);

	// Output to PBO
	orbitOut = orbit;

//-----------------------------------------------------------------------------
// To calculate depth for use with rasterized material
//-----------------------------------------------------------------------------
	vec4 pClipSpace =  MVEPMat * vec4(pos, 1.0);
	vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	float ndcDepth = pNdc.z;
	
	float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	gl_FragDepth = d;

}
