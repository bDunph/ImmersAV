#version 410  
// raymarch basic setup adapted from dila's tutorial
// https://www.youtube.com/watch?v=yxNnRSefK94

#define MAX_ITERATIONS 500
#define SUN_DIR vec3(0.5, 0.8, 0.0)
#define EPSILON 0.001
#define PLANE_NORMAL vec4(0.0, 1.0, 0.0, 0.0)
#define PLANE_ID 0
#define MANDEL_ID 1
#define SKY_ID 2

uniform mat4 MVEPMat;

in vec4 nearPos;
in vec4 farPos;

layout(location = 0) out vec4 fragColor; 
layout(location = 1) out vec4 dataOut;

int index;
float mandelDist, planeDist;

//----------------------------------------------------------------------------------------
// Mandelbulb SDF adapted from https://www.shadertoy.com/view/tdtGRj and based on 
// http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
//----------------------------------------------------------------------------------------
float mandelbulbSDF(vec3 pos) {

	float Power = 8.0;

    	float r = length(pos);
    	if(r > 1.5) return r-1.2;

    	vec3 z = pos;
    	float dr = 1.0, theta, phi;

    	for (int i = 0; i < 10; i++) {
    	    	r = length(z);
    	    	if (r>1.5) break;

		// convert to polar coordinates
    	    	theta = acos(z.y/r);
     	    	phi = atan(z.z,z.x);

		// length of the running complex derivative
    	    	//dr =  pow(r, Power-1.0)*Power*dr*(0.7+lowFreqVal*fftBinValScale) + 1.0;
    	    	dr =  pow(r, Power-1.0)*Power*dr + 1.0;

		// scale and rotate 
    	    	theta *= Power;
    	    	phi *= Power;

		// mandelbulb formula with y and z terms swapped to rotate the object
    	    	z = pow(r,Power)*vec3(sin(theta)*cos(phi), cos(theta), sin(phi)*sin(theta)) + pos;
    	    }

    	return abs(0.5*log(r)*r/dr);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Ground plane SDF from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//----------------------------------------------------------------------------------------
float planeSDF(vec3 pos, vec4 normal){
	
	return dot(pos, normal.xyz) + normal.w;
}

float sceneSDF(vec3 p)
{
	vec3 newPos = p;
	float function1x = 0.09*sin(newPos.x*0.4)*newPos.x;
	float function1z = 0.09*sin(newPos.z*0.4)*newPos.z;

	float function2x = clamp(function1x, 0.0, 3.0);
	float function2z = clamp(function1z, 0.0, 3.0);

	newPos.y += function2x;
	newPos.y += function2z;

	planeDist = planeSDF(newPos, PLANE_NORMAL);

	mandelDist = mandelbulbSDF(p + vec3(0.0, -1.0, 0.0));

	return min(planeDist, mandelDist);
}

vec2 march(vec3 origin, vec3 direction)
{
 	float depth = 0.0;
	int ind = 0;
	float id;
    	for(int i = 0; i < MAX_ITERATIONS; ++i)
    	{
    	 	vec3 p = origin + direction * depth;

    	    float d = sceneSDF(p);

    	    if(d < EPSILON) 
			{
				if(d == mandelDist)
				{
					id = MANDEL_ID;		
				} 
				else if(d == planeDist)
				{
					id = PLANE_ID;
				}
				break;
			}
			depth += d * 0.99;
			id = SKY_ID;
    	}
    	return vec2(depth, id);
}

// finite difference normal from 
// http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-ii-lighting-and-coloring/
vec3 norm(vec3 pos, vec3 dir)
{
	return normalize(vec3(	sceneSDF(pos + vec3(EPSILON, 0.0, 0.0)) - sceneSDF(pos - vec3(EPSILON, 0.0, 0.0)),
                			sceneSDF(pos + vec3(0.0, EPSILON, 0.0)) - sceneSDF(pos - vec3(0.0, EPSILON, 0.0)),
                			sceneSDF(pos + vec3(0.0, 0.0, EPSILON)) - sceneSDF(pos - vec3(0.0, 0.0, EPSILON))));
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
	//vec2 dist = march(rayOrigin + vec3(0.0, 2.0, 0.0), rayDir);
	vec2 dist = march(rayOrigin, rayDir);
	
	vec3 pos = rayOrigin + dist.x * rayDir;// + (noiseCalc * 0.01);

	// material colour
	
	vec3 colour = vec3(0.0); 
	vec3 totMatCol = vec3(0.0);

	// colouring and shading
	vec3 norm = norm(pos, rayDir);

	if(dist.y == MANDEL_ID)
	{
		totMatCol = vec3(0.12, 0.075, 0.002);
		
		// lighting algorithm from https://www.iquilezles.org/www/articles/outdoorslighting/outdoorslighting.htm
		float sun = clamp(dot(norm, -SUN_DIR), 0.0, 1.0);
		float sky = clamp(0.5 + 0.5 * norm.y, 0.0, 1.0);
		float ind = clamp(dot(norm, normalize(-SUN_DIR * vec3(1.0, 0.0, 1.0))), 0.0, 1.0);
		    
		vec3 lightRig = sun * vec3(1.64, 1.27, 0.99);
		lightRig += sky * vec3(0.16, 0.2, 0.28);
		lightRig += ind * vec3(0.4, 0.28, 0.2);

		colour = totMatCol * lightRig;
	}
	else if(dist.y == PLANE_ID)
	{
		totMatCol = vec3(0.04, 0.125, 0.125);
		
		// lighting algorithm from https://www.iquilezles.org/www/articles/outdoorslighting/outdoorslighting.htm
		float sun = clamp(dot(norm, -SUN_DIR), 0.0, 1.0);
		float sky = clamp(0.5 + 0.5 * norm.y, 0.0, 1.0);
		float ind = clamp(dot(norm, normalize(-SUN_DIR * vec3(1.0, 0.0, 1.0))), 0.0, 1.0);
		    
		vec3 lightRig = sun * vec3(1.64, 1.27, 0.99);
		lightRig += sky * vec3(0.16, 0.2, 0.28);
		lightRig += ind * vec3(0.4, 0.28, 0.2);

		colour = totMatCol * lightRig;
	}
	else if(dist.y == SKY_ID)
	{
		colour = vec3(0.235, 0.385, 0.46);
	}

	// gamma corr
	colour = pow(colour, vec3(1.0/2.2));
	
	// Output to screen
	fragColor = vec4(colour,1.0);

	// Output to PBO
	//dataOut = fragColor;

//-----------------------------------------------------------------------------
// To calculate depth for use with rasterized material e.g. VR controllers
//-----------------------------------------------------------------------------
	//vec4 pClipSpace =  MVEPMat * vec4(pos, 1.0);
	//vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	//float ndcDepth = pNdc.z;

	//float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	//gl_FragDepth = d;

}
