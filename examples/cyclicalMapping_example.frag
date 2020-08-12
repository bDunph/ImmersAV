#version 410  
// raymarch basic setup adapted from dila's tutorial
// https://www.youtube.com/watch?v=yxNnRSefK94

#define MAX_ITERATIONS 100
#define SUN_DIR vec3(0.5, 0.8, 0.0)
#define EPSILON 0.01

uniform mat4 MVEPMat;
uniform float sineControlVal;
uniform float centOut;

in vec4 nearPos;
in vec4 farPos;

layout(location = 0) out vec4 fragColor; 
layout(location = 1) out vec4 dataOut;

int index;

//----------------------------------------------------------------------------------------
// Sphere SDF from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//----------------------------------------------------------------------------------------
float sphereSDF(vec3 p, float radius)
{
	return length(p) - radius;
}

float DE(vec3 p)
{
	p.y -= 1.0;
	//float rad = 1.0 * abs(sineControlVal);
	//float rad = 1.0 * abs(centOut * 0.001);
	float rad = 1.0 * (1.0 / (centOut * 0.01));

	float sphereDist = sphereSDF(p, rad);

	return sphereDist;
}

float march(vec3 o, vec3 r)
{
 	float t = 0.0;
	int ind = 0;
    	for(int i = 0; i < MAX_ITERATIONS; ++i)
    	{
    	 	vec3 p = o + r * t;

    	    	float d = DE(p);

    	    	if(d < EPSILON) break;
		t += d * 0.5;
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
	
	vec3 pos = rayOrigin + dist * rayDir;// + (noiseCalc * 0.01);

	// material colour
	
	vec3 colour = vec3(0.0);
	vec3 totMatCol = vec3(0.0);

	if(index < MAX_ITERATIONS)
	{
		// colouring and shading
		vec3 norm = norm(pos, rayDir);

		totMatCol = vec3(1.0, 0.0, 0.0);

		// lighting
		//float ao = ao(pos, norm, 0.5, 5.0);
		float sun = clamp(dot(norm, SUN_DIR), 0.0, 1.0);
		float sky = clamp(0.5 + 0.5 * norm.y, 0.0, 1.0);
		float ind = clamp(dot(norm, normalize(SUN_DIR * vec3(-1.0, 0.0, -1.0))), 0.0, 1.0);
			    
		vec3 lightRig = sun * vec3(1.64, 1.27, 0.99);
		lightRig += sky * vec3(0.16, 0.2, 0.28);// * ao;
		lightRig += ind * vec3(0.4, 0.28, 0.2);// * ao;

		colour = totMatCol * lightRig;
	}

	// gamma corr
	colour = pow(colour, vec3(1.0/2.2));
	
	// Output to screen
	fragColor = vec4(colour,1.0);

	// Output to PBO
	dataOut = fragColor;

//-----------------------------------------------------------------------------
// To calculate depth for use with rasterized material e.g. VR controllers
//-----------------------------------------------------------------------------
	//vec4 pClipSpace =  MVEPMat * vec4(pos, 1.0);
	//vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	//float ndcDepth = pNdc.z;
	//
	//float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	//gl_FragDepth = d;

}
