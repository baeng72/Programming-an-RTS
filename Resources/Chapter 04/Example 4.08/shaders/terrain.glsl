#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=3) in vec2 inAlphaUV;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec2 outUV;
layout(location=2) out vec2 outAlphaUV;


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
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outUV = inUV;
	outAlphaUV = inAlphaUV;
}

#type fragment
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec2 inUV;
layout(location=2) in vec2 inAlphaUV;


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

layout (set=1,binding=0) uniform sampler2D texmap1;
layout (set=1,binding=1) uniform sampler2D texmap2;
layout (set=1,binding=2) uniform sampler2D texmap3;
layout (set=1,binding=3) uniform sampler2D alphamap;

void main(){
	
	
	vec4 inColor1 = texture(texmap1,inUV);
	vec4 inColor2 = texture(texmap2,inUV);
	vec4 inColor3 = texture(texmap3,inUV);
	
	//alphamap determines how much is shown
	vec4 alphaColor = texture(alphamap,inAlphaUV);
	//Calculate the inverse
	float inverse = 1.0f / (alphaColor.r + alphaColor.g + alphaColor.b);
	inColor1 *= alphaColor.b * inverse;
	inColor2 *= alphaColor.g * inverse;
	inColor3 *= alphaColor.r * inverse;
	vec4 combocol = inColor1+inColor2+inColor3;
	//normalize interpolated normal
	vec3 normal = normalize(inNormal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(combocol)*vec3(light.ambient);
	vec3 diffuse = vec3(combocol)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(combocol) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1);
	outFragColor = color;	
}