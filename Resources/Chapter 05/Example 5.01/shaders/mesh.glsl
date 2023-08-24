#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;


layout(location=0) out vec3 outNormal;
layout(location=1) out vec4 outColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
	DirectionalLight light;
	
};

layout (push_constant) uniform PushConst{	
	mat4 model;		//varies per object
	vec4 color;
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outColor = color;
}

#type fragment
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec4 inColor;

layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
	DirectionalLight light;
};


void main(){
	//normalize interpolated normal
	vec3 normal = normalize(inNormal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1.0);
	outFragColor = color;	
}