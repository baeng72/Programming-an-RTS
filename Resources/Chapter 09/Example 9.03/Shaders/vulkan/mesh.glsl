#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;



layout(location=0) out vec3 outNormal;
layout(location=1) out vec2 outUV;
layout(location=2) out vec2 outWorldUV;



struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
	DirectionalLight light;
	vec2 mapSize;
};

layout (push_constant) uniform PushConst{	
	mat4 model;		//varies per object		
};

void main(){
	vec4 worldPos = model * vec4(inPos,1.0);
	gl_Position = viewProj * worldPos;
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outUV = inUV;
	outWorldUV = vec2(worldPos.x / mapSize.x, -worldPos.z/mapSize.y);
}

#type fragment
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec2 inUV;
layout(location=2) in vec2 inWorldUV;


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

layout (set=1,binding=0) uniform sampler2D texmap;
layout (set=2,binding=0) uniform sampler2D lightmap;

layout (push_constant) uniform PushConst{	
	mat4 model;		//varies per object
	
};


void main(){
	//lightmap 
	float lm  = texture(lightmap, inWorldUV).r;
	
	vec4 inColor = texture(texmap,inUV)*lm;
	
	
	//normalize interpolated normal
	vec3 normal = normalize(inNormal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 clr = vec4((ambient + diffuse + spec) ,1.0);	
	outFragColor = clr;
}