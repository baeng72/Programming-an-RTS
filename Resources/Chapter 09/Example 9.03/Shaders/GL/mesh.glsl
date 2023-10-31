#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;



layout(location=0) out vec3 aNormal;
layout(location=1) out vec2 aUV;
layout(location=2) out vec2 aWorldUV;



uniform	mat4 viewProj;	//same whole scene
uniform	vec2 mapSize;


uniform mat4 model;		//varies per object		


void main(){
	vec4 worldPos = model * vec4(inPos,1.0);
	gl_Position = viewProj * worldPos;
	aNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	aUV = inUV;
	aWorldUV = vec2(worldPos.x / mapSize.x, -worldPos.z/mapSize.y);
}

#type fragment
#version 460 core
layout(location=0) in vec3 aNormal;
layout(location=1) in vec2 aUV;
layout(location=2) in vec2 aWorldUV;


layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

uniform	DirectionalLight light;


uniform sampler2D texmap;
uniform sampler2D lightmap;




void main(){
	//lightmap 
	float lm  = texture(lightmap, aWorldUV).r;
	
	vec4 inColor = texture(texmap,aUV)*lm;
	
	
	//normalize interpolated normal
	vec3 normal = normalize(aNormal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 clr = vec4((ambient + diffuse + spec) ,1.0);	
	outFragColor = clr;
}