#type vertex
#version 460 core

layout (location=0) in vec3 inPos;
layout (location=1) in vec2 inUV;

layout (location=0) out vec2 outUV;


layout (location=0) uniform mat4 matVP;
layout (location=1) uniform	mat4 matWorld;




void main(){	
	vec4 pos = matVP * matWorld * vec4(inPos,1.0);
	gl_Position = vec4(pos.x,pos.y,pos.z,pos.w);
	outUV=inUV;
	
}

#type fragment
#version 460 core

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout(binding=0) uniform sampler2D sightTexture;


void main(){		
	vec4 color = vec4(1,1,1,texture(sightTexture,inUV).r);	
	outFragColor = color;
}