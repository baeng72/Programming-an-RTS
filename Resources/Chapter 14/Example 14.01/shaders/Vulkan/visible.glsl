#type vertex
#version 450

layout (location=0) in vec3 inPos;
layout (location=1) in vec2 inUV;

layout (location=0) out vec2 outUV;


layout (push_constant) uniform PushConstants{
	mat4 matVP;
	mat4 matWorld;
};



void main(){	
	
	gl_Position = matVP * matWorld * vec4(inPos,1.0);
	outUV=inUV;
	
}

#type fragment
#version 450

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout (binding=0) uniform sampler2D sightTexture;


void main(){		
	vec4 color = vec4(1,1,1,texture(sightTexture,inUV).r);	
	outFragColor = color;
}