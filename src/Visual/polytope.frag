#version 410

#define MaxSteps 80
#define MinimumDistance 0.0009
#define normalDistance     0.0002

#define Scale 3.0
#define FieldOfView 0.5
#define Jitter 0.06
#define FudgeFactor 1.0

#define Ambient 0.3
#define Diffuse 2.0
#define LightDir vec3(1.0)
#define LightColor vec3(0.3,0.3,0.3)
#define LightDir2 vec3(1.0,-1.0,1.0)
#define LightColor2 vec3(0.2,0.2,0.2)
#define Offset vec3(0.92858,0.92858,0.32858)

#define uD 2.0
#define uAO 0.04 
 
vec3 lightDir = LightDir;
vec3 lightDir2 = LightDir2;
vec3 spotDir = LightDir2;

// control-group: style
float Refraction = 1.0f; // control[1, 0.01-1]

//uniform vec2 resolution;
//uniform vec3 cameraPosition;
//uniform mat4 cameraWorldMatrix;
//uniform mat4 cameraProjectionMatrixInverse;

vec3 pos4Depth;

uniform mat4 proj;
uniform mat4 eyeMat;
uniform mat4 view;
uniform float rot3D;
uniform float timer;

in vec4 eye;
in vec4 worldDir;
in vec4 cameraForward;

out vec4 fragColor;

// Original DE from Knighty's Fragmentarium frag: http://www.fractalforums.com/fragmentarium/solids-many-many-solids/

#define PI 3.141592

// Return rotation matrix for rotating around vector v by angle
mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

vec2 rotate(vec2 v, float a) {
	return vec2(cos(a)*v.x + sin(a)*v.y, -sin(a)*v.x + cos(a)*v.y);
}

// control-group: coordinate
int Degree = 3; // control[4, 3-5]
float U = 1.0; // control[0, 0-1]
float V = 0.0; // control[0, 0-1]
float W = 0.0; // control[0, 0-1]
float T = 0.0; // control[1, 0-1]

// control-group: rotation
float RotationX = 1.0; // control[1, 0-1]
float RotationY = 0.0; // control[0, 0-1]
float RotationZ = 0.0; // control[0, 0-1]
float Angle = 90.0; // control[0, 0-360]

// control-group: style
float VRadius = 0.04; // control[0.04, 0-0.2]
float SRadius = 0.03; // control[0.03, 0-0.2]
 
bool displaySegments = true; // control[true]
bool displayVertices = true; // control[true]

mat3 rot;
vec4 nc,nd,p;
// Sets up the 4 generator planes and calculates the position of the initial vertex as a vec4 p. 
// Also defines a rotation matrix in 3D.
void init() {

	float cospin=cos(PI/float(Degree)), isinpin=1./sin(PI/float(Degree));
	float scospin=sqrt(2./3.-cospin*cospin), issinpin=1./sqrt(3.-4.*cospin*cospin);

	nc=0.5*vec4(0,-1,sqrt(3.),0.);
	nd=vec4(-cospin,-0.5,-0.5/sqrt(3.),scospin);

	vec4 pabc,pbdc,pcda,pdba;
	pabc=vec4(0.,0.,0.,1.);
	pbdc=0.5*sqrt(3.)*vec4(scospin,0.,0.,cospin);
	pcda=isinpin*vec4(0.,0.5*sqrt(3.)*scospin,0.5*scospin,1./sqrt(3.));
	pdba=issinpin*vec4(0.,0.,2.*scospin,1./sqrt(3.));
	
	p=normalize(V*pabc+U*pbdc+W*pcda+T*pdba);
	
	float dynamicAngle = mod(timer, 360.0);

	rot = rotationMatrix3(normalize(vec3(RotationX,RotationY,RotationZ)), dynamicAngle);//in reality we need a 4D rotation
}

vec4 fold(vec4 pos) {
	for(int i=0;i<25;i++){
		vec4 tmp = pos;
		pos.xy=abs(pos.xy);
		float t=-2.*min(0.,dot(pos,nc)); pos+=t*nc;
		t=-2.*min(0.,dot(pos,nd)); pos+=t*nd;
		if (tmp == pos) { return pos; }
	}
	return pos;
}

float DD(float ca, float sa, float r){
	//magic formula to convert from spherical distance to planar distance.
	//involves transforming from 3-plane to 3-sphere, getting the distance
	//on the sphere then going back to the 3-plane.
	return r-(2.*r*ca-(1.-r*r)*sa)/((1.-r*r)*ca+2.*r*sa+1.+r*r);
}

float dist2Vertex(vec4 z, float r){
	float ca=dot(z,p), sa=0.5*length(p-z)*length(p+z);//sqrt(1.-ca*ca);//
	return DD(ca,sa,r)-VRadius;
}

float dist2Segment(vec4 z, vec4 n, float r){
	//pmin is the orthogonal projection of z onto the plane defined by p and n
	//then pmin is projected onto the unit sphere
	float zn=dot(z,n),zp=dot(z,p),np=dot(n,p);
	float alpha=zp-zn*np, beta=zn-zp*np;
	vec4 pmin=normalize(alpha*p+min(0.,beta)*n);
	//ca and sa are the cosine and sine of the angle between z and pmin. This is the spherical distance.
	float ca=dot(z,pmin), sa=0.5*length(pmin-z)*length(pmin+z);//sqrt(1.-ca*ca);//
	float factor = 1.0;// DD(ca,sa,r)/DD(ca+0.01,sa,r);
	return (DD(ca,sa,r)-SRadius*factor)*min(1.0/factor,1.0);
	
}
//it is possible to compute the distance to a face just as for segments: pmin will be the orthogonal projection
// of z onto the 3-plane defined by p and two n's (na and nb, na and nc, na and and, nb and nd... and so on).
//that involves solving a system of 3 linear equations.
//it's not implemented here because it is better with transparency

float dist2Segments(vec4 z, float r){
	float da=dist2Segment(z, vec4(1.,0.,0.,0.), r);
	float db=dist2Segment(z, vec4(0.,1.,0.,0.), r);
	float dc=dist2Segment(z, nc, r);
	float dd=dist2Segment(z, nd, r);
	
	return min(min(da,db),min(dc,dd));
}

float DE(vec3 pos) {
	
	float r=length(pos);//by multiplying by 1.x you can get incomplete shapes
	vec4 z4=vec4(2.*pos,1.-r*r)*1./(1.+r*r);//Inverse stereographic projection of pos: z4 lies onto the unit 3-sphere centered at 0.
	z4.xyw=rot*z4.xyw;
	z4=fold(z4);//fold it
	float d=10000.;
	if(displayVertices ) d=min(d,dist2Vertex(z4,r));
	if(displaySegments) d=min(d,dist2Segments(z4, r));
	return d ;
}

vec4 baseColor(vec3 pos, vec3 normal){
	return vec4(0.0, 0.0, 0.0, 1.0);
}
// Two light sources plus specular 
vec3 getLight(in vec3 color, in vec3 normal, in vec3 dir) {
	float diffuse = max(0.0,dot(normal, lightDir)); // Lambertian
	
	float diffuse2 = max(0.0,dot(normal, lightDir2)); // Lambertian
	

	vec3 r = spotDir - 2.0 * dot(normal, spotDir) * normal;
	float s = max(0.0,dot(dir,-r));
	
	vec3 r2 = vec3(-1,0,0) - 2.0 * dot(normal, vec3(-1,0,0)) * normal;
	float s2 = max(0.0,dot(dir,-r2));
	

	return
	
	(diffuse*Diffuse)*(LightColor*color) +
	(diffuse2*Diffuse)*(LightColor2*color) +pow(s,20.0)*vec3(0.3)+pow(s2,120.0)*vec3(0.3);
}

// Finite difference normal
vec3 getNormal(in vec3 pos) {
	vec3 e = vec3(0.0,normalDistance,0.0);
	
	return normalize(vec3(
			DE(pos+e.yxx)-DE(pos-e.yxx),
			DE(pos+e.xyx)-DE(pos-e.xyx),
			DE(pos+e.xxy)-DE(pos-e.xxy)
			)
		);
}

// Solid color 
vec3 getColor(vec3 normal, vec3 pos) {
	return vec3(0.2,0.13,0.94);
}


// Pseudo-random number
// From: lumina.sourceforge.net/Tutorials/Noise.html
float rand(vec2 co){
	return fract(cos(dot(co,vec2(4.898,7.23))) * 23421.631);
}

// Ambient occlusion approximation.
// Sample proximity at a few points in the direction of the normal.
float ambientOcclusion(vec3 p, vec3 n) {
	float ao = 0.0;
	float de = DE(p);
	float wSum = 0.0;
	float w = 1.0;
    float d = uD;
    float aoEps = uAO; // 0.04;
	for (float i =1.0; i <6.0; i++) {
		// D is the distance estimate difference.
		// If we move 'n' units in the normal direction,
		// we would expect the DE difference to be 'n' larger -
		// unless there is some obstructing geometry in place
		float D = (DE(p+ d*n*i*i*aoEps) -de)/(d*i*i*aoEps);
		w *= 0.6;
		ao += w*clamp(1.0-D,0.0,1.0);
		wSum += w;
	}
	return clamp(ao/wSum, 0.0, 1.0);
}


vec4 getMyColor(in vec3 pos,in vec3 normal, vec3 dir) {
	float ao = ambientOcclusion(pos,normal)*0.4;	
	vec4 color = baseColor(pos,normal);
	vec3 light = getLight(color.xyz, normal, dir);
	color.xyz = mix(color.xyz*Ambient+light,vec3(0),ao);
	return color;
}


vec4 rayMarch(in vec3 from, in vec3 dir) {
	// Add some noise to prevent banding
	float totalDistance = 0.;//Jitter*rand(fragCoord.xy+vec2(iTime));
	vec3 dir2 = dir;
	float distance;
	int steps = 0;
	vec3 pos;
	vec3 bestPos;
	float bestDist = 1000.0;
	float bestTotal = 0.0;
	vec3 acc = vec3(0.0);
	float rest = 1.0;
	
	float minDist = 0.0;
	float ior = Refraction;
	for (int i=0; i <= MaxSteps; i++) {
		pos = from + totalDistance * dir;
		distance = abs(DE(pos / 0.35) * 0.35)*FudgeFactor;
		if (distance<bestDist) {
			bestDist = distance;
			bestPos = pos;
			bestTotal = totalDistance;
		}
		
		totalDistance += distance;
		
		if (distance < MinimumDistance && distance>minDist) {
		    minDist = distance;
			vec3 normal = getNormal(pos-dir*normalDistance*3.0);
			vec4 c = getMyColor(pos-dir*normalDistance*3.0,normal,dir);
			
			acc+=rest*c.xyz*c.w;
			rest*=(1.0-c.w);
			
			if (rest<0.1) break;
			
			
			totalDistance += 0.05;
			
			from = from + totalDistance * dir;
			totalDistance = 0.0;
			if (dot(dir,normal)>0.0) normal*=-1.0;
			dir = refract(dir, normal, ior);
			ior = 1.0/ior;
			bestDist = 1000.0;	
		}
		
		if (distance>minDist) minDist = 0.0;
		steps = i;
	}

	if (steps == MaxSteps) {
		pos = bestPos;
		return vec4(1.0, 0.9804, 0.9412, 1.0); 	
		//vec3 normal = getNormal(pos-dir*normalDistance*3.0);
		//vec4 c = getMyColor(pos,normal,dir);
		//acc += rest*mix(c.xyz,vec3(1),min(bestDist/bestTotal*400.,1.0));
	}
	
	pos4Depth = pos;

	return vec4(pow(acc,vec3(0.6,0.5,0.5)),1.0);
} 


vec2 uv;
float rand(float c){
	return rand(vec2(c,1.0));
}

void main(void) {

	// This is taken from: https://raw.githubusercontent.com/mrdoob/three.js/master/examples/webgl_raymarching_reflect.html

	//float rot3DRad = rot3D * PI / 180.0;
	//vec3 eyePos = vec3(eye.x * cos(rot3DRad), eye.y * sin(rot3DRad), eye.z);
	vec3 eyePos = vec3(eye.x, eye.y - 1.0, eye.z);
	vec3 ray = normalize(worldDir.xyz);

	lightDir = normalize(eyePos + vec3(1,1,1));
	lightDir2 = normalize(vec3(-1,0,-1));
	spotDir = normalize(vec3(-0.,-1.,-1.));

	init();

	fragColor = vec4( rayMarch(eyePos, ray));
	
	vec4 pClipSpace = proj * eyeMat * view * vec4(pos4Depth, 1.0);
	vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	float ndcDepth = pNdc.z;

	float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	gl_FragDepth = d;
}
