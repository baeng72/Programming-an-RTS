#type vertex
#version 460 core
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNorm;
layout (location=2) in vec2 inUV;

layout (location=0) out vec2 aUV;


uniform	mat4 matVP;


uniform	mat4 matWorld;



void main(){
	gl_Position = matVP * matWorld * vec4(inPos,1.0);
	aUV = inUV;
}

#type fragment
#version 460 core
layout(location=0) in vec2 aUV;
layout(location=0) out vec4 outFragColor;

uniform vec4 color;
uniform sampler2D effectTexture;

void main(){
	vec4 clr = texture(effectTexture,aUV);
	outFragColor = color * clr;
}