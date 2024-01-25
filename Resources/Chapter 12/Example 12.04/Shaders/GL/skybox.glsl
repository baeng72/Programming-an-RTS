#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUV;

layout(location=0) out vec2 outUV;



uniform	mat4 viewProj;	//same whole scene


uniform	mat4 world;


void main(){
	gl_Position = viewProj * world * vec4(inPos,1.0);
	outUV = inUV;
}

#type fragment
#version 460 core
layout(location=0) in vec2 inUV;
layout(location=0) out vec4 outFragColor;

uniform sampler2D texmap;


void main(){
	vec4 clr = texture(texmap,inUV);
	outFragColor=clr;	
}