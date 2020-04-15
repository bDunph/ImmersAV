#version 410

uniform vec3 lightPos;
uniform vec3 light2Pos;
uniform float alpha;
uniform vec3 camPos;
uniform samplerCube skybox;
 
in vec3 vertNormal_worldSpace;
in vec3 fragPos_worldSpace;

out vec4 colour_out;

void main() {

//************ Refraction***************************//
	// ratio of air refractive index 1.0 and glass refractive index 1.52
	//float ratio = 1.0 / 1.52;
	//vec3 I = normalize(fragPos_worldSpace - camPos);
	//vec3 R = refract(I, normalize(vertNormal_worldSpace), ratio);
	//colour_out = vec4(texture(skybox, R).rgb, 1.0);
//*********************************************//

//*********** Blinn-Phong ************************//	
	float specularStrength = 0.4;
	vec3 lightColour = vec3(1.0, 1.0, 1.0);
	
	//*** Ambient ***//
	vec3 objectColour = vec3(0.5, 0.125, 0.05);
	float ambientStrength = 0.3;

	vec3 ambient = ambientStrength * lightColour;

	//*** Diffuse ***//
	vec3 norm = normalize(vertNormal_worldSpace);
	vec3 lightDir_worldSpace = normalize(lightPos- fragPos_worldSpace);
	vec3 light2Dir_worldSpace = normalize(light2Pos - fragPos_worldSpace);
	float diffuseAngle = max(dot(norm, lightDir_worldSpace), 0.0);
	float diffuseAngle2 = max(dot(norm, light2Dir_worldSpace), 0.0);
	vec3 diffuse = diffuseAngle * lightColour;
	vec3 diffuse2 = diffuseAngle2 * lightColour;

	//*** Specular ***//
	vec3 viewDir = normalize(camPos - fragPos_worldSpace);
	vec3 halfWay = normalize(lightDir_worldSpace + viewDir);
	vec3 halfWay2 = normalize(light2Dir_worldSpace + viewDir);
	//vec3 reflectDir = reflect(-lightDir_worldSpace, norm);
	float specAngle = pow(max(dot(norm, halfWay), 0.0), 16.0);
	float specAngle2 = pow(max(dot(norm, halfWay2), 0.0), 16.0);
	vec3 specular = specularStrength * specAngle * lightColour;
	vec3 specular2 = specularStrength * specAngle2 * lightColour;

	vec3 result = (ambient + diffuse + specular) * objectColour;
	vec3 result2 = (ambient + diffuse2 + specular2) * objectColour;

	colour_out = vec4(result + result2, alpha);
//****************************************************//
}
