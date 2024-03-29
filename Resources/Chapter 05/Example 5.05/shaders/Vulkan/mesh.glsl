#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;

layout(location=0) out vec3 outNormal;




struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	
	DirectionalLight light;
};

layout (push_constant) uniform PushConst{	//this may not work on some devices (128byte max size)
	mat4 viewProj;
	mat4 model;
	vec4 clrdiffuse;
	vec4 clrspecular;
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	
}

#type fragment
#version 450
layout(location=0) in vec3 inNormal;

layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{	
	DirectionalLight light;
};

layout (push_constant) uniform PushConst{		
	mat4 viewProj;	
	mat4 model;
	vec4 clrdiffuse;
	vec4 clrspecular;
};



void main(){
	
	
	vec4 inAmbient = clrdiffuse;
	vec4 inDiffuse = clrdiffuse;
	vec4 inSpecular = clrspecular;
	
	
	//normalize interpolated normal
	vec3 normal = normalize(inNormal);
	float shade = max(dot(normal,light.direction),0);
	vec3 ambient = vec3(inAmbient)*vec3(light.ambient);
	vec3 diffuse = vec3(inDiffuse)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inSpecular) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1);
	outFragColor = color;	
}