#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=3) in ivec4 inBoneIDs;
layout(location=4) in vec4 inBoneWeights;


layout(location=0) out vec3 Normal;
layout(location=1) out vec2 UV;



uniform	mat4 viewProj;	//same whole scene
	
layout(binding=0) readonly buffer skeleton{
	mat4 bones[];
};


uniform	mat4 model;		//varies per object
	
void main(){
	mat4 skin = bones[inBoneIDs.x]*inBoneWeights.x +
		bones[inBoneIDs.y]*inBoneWeights.x +
		bones[inBoneIDs.z]* inBoneWeights.z +
		inBoneWeights.w * bones[inBoneIDs.w];
	gl_Position = viewProj *  model * skin * vec4(inPos,1.0);
	Normal = vec3(inverse(skin*model) * vec4(inNormal,0.0));
	UV = inUV;	
}

#type fragment
#version 460 core
layout(location=0) in vec3 Normal;
layout(location=1) in vec2 UV;



layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

uniform	DirectionalLight light;




uniform	vec4 color;


uniform sampler2D texmap;


void main(){
	
	
	vec4 inColor = texture(texmap,UV);
	float Inv = 1.0f - inColor.a;
	
	//normalize interpolated normal
	vec3 normal = normalize(Normal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 finalColor = vec4((ambient + diffuse + spec)*Inv + vec3(color) * inColor.a,1.0);
	outFragColor = finalColor;	
}