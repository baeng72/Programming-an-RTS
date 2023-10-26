#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;


layout(location=0) out vec3 Normal;




uniform	mat4 viewProj;	//same whole scene


uniform	mat4 model;		//varies per object



void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	Normal = vec3(inverse(model) * vec4(inNormal,0.0));

}

#type fragment
#version 460 core
layout(location=0) in vec3 Normal;


layout(location=0) out vec4 outFragColor;

uniform	vec4 inColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

uniform DirectionalLight light;



void main(){
	//normalize interpolated normal
	vec3 normal = normalize(Normal);
	float shade = max(dot(normal,light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1.0);
	outFragColor = color;	
}