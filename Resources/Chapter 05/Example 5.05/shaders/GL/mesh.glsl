#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;

out vec3 aNormal;







uniform	mat4 viewProj;
uniform	mat4 model;



void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	aNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	
}

#type fragment
#version 460 core
layout(location=0) in vec3 aNormal;

layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

uniform	DirectionalLight light;



uniform	vec4 clrdiffuse;
uniform	vec4 clrspecular;




void main(){
	
	
	vec4 inAmbient = clrdiffuse;
	vec4 inDiffuse = clrdiffuse;
	vec4 inSpecular = clrspecular;
	
	
	//normalize interpolated normal
	vec3 normal = normalize(aNormal);
	float shade = max(dot(normal,light.direction),0);
	vec3 ambient = vec3(inAmbient)*vec3(light.ambient);
	vec3 diffuse = vec3(inDiffuse)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inSpecular) * vec3(light.specular) * specfactor ;

	outFragColor = vec4(ambient + diffuse + spec,1);	
}