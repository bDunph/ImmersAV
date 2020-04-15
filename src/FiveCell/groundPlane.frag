#version 410

const float GAMMA = 2.2;

struct Light{
	
	vec3 direction;
	vec3 colour;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

struct Material{

	sampler2D groundTex;
	vec3 specular;
	float shininess;
};

uniform Material material;

uniform vec3 camPos;

//uniform sampler2D groundTex;

in vec3 fragPos_worldSpace;
in vec3 normal_worldSpace;
in vec2 texCoordOut;

out vec4 colour_out;

void main(){

	//*** Ambient ***//

	vec3 ambient = light.ambient * light.colour * texture(material.groundTex, texCoordOut).rgb;

	//*** Diffuse ***//

	vec3 norm = normalize(normal_worldSpace);
	vec3 lightDir_worldSpace = normalize(-light.direction);
	float diffuseAngle = max(dot(norm, lightDir_worldSpace), 0.0);
	vec3 diffuse = light.diffuse * light.colour * diffuseAngle * texture(material.groundTex, texCoordOut).rgb;;

	//*** Specular ***//

	vec3 viewDir = normalize(camPos - fragPos_worldSpace);
	vec3 halfWay = normalize(lightDir_worldSpace + viewDir);
	float specAngle = pow(max(dot(norm, halfWay), 0.0), material.shininess);
	vec3 specular = light.specular * light.colour * (specAngle * material.specular);

	vec3 result = ambient + diffuse + specular;

	//gamma correction
	vec3 colWithGamma = pow(result, vec3(1.0 / GAMMA));

    	colour_out = vec4(colWithGamma, 1.0);
} 
