#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=3) in vec2 inAlphaUV;

layout(location=0) out vec3 Normal;
layout(location=1) out vec2 UV;
layout(location=2) out vec2 AlphaUV;


uniform	mat4 viewProj;	//same whole scene
	

uniform	mat4 model;		//varies per object


void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	Normal = vec3(inverse(model) * vec4(inNormal,0.0));
	UV = inUV;
	AlphaUV = inAlphaUV;
}


#type fragment
#version 450
layout(location=0) in vec3 Normal;
layout(location=1) in vec2 UV;
layout(location=2) in vec2 AlphaUV;


layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

uniform	DirectionalLight light;


uniform sampler2D a_texmap1;
uniform sampler2D b_texmap2;
uniform sampler2D c_texmap3;
uniform sampler2D d_alphamap;
uniform sampler2D e_lightmap;

void main(){
	
	
	vec4 inColor1 = texture(a_texmap1,UV);
	vec4 inColor2 = texture(b_texmap2,UV);
	vec4 inColor3 = texture(c_texmap3,UV);
	
	
	//alphamap determines how much is shown
	vec4 alphaColor = texture(d_alphamap,AlphaUV);
	
	//lightmap 
	float lm  = texture(e_lightmap, AlphaUV).r;
	//Calculate the inverse
	float inverse = 1.0f / (alphaColor.r + alphaColor.g + alphaColor.b);
	inColor1 *= alphaColor.b * inverse *lm ;
	inColor2 *= alphaColor.g * inverse *lm ;
	inColor3 *= alphaColor.r * inverse *lm ;
	vec4 combocol = inColor1+inColor2+inColor3;
	
	//normalize interpolated normal
	vec3 normal = normalize(Normal);
	float shade = max(dot(normal,light.direction),0);
	vec3 ambient = vec3(combocol)*vec3(light.ambient);
	vec3 diffuse = vec3(combocol)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(combocol) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1.0);
	outFragColor = color;	
}