#type vertex
#version 450
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNorm;
layout (location=2) in vec2 inUV;

layout (location=0) out vec2 outUV;

layout(binding=0) uniform UBO{
	mat4 matVP;
};

layout(push_constant) uniform PushConst{
	mat4 matWorld;
	vec3 color;
};

void main(){
	gl_Position = matVP * matWorld * vec4(inPos,1.0);
	outUV = inUV;
}

#type fragment
#version 450
layout(location=0) in vec2 inUV;
layout(location=0) out vec4 outFragColor;

layout(push_constant) uniform PushConst{
	mat4 matWorld;
	vec4 color;
};

layout (binding=1) uniform sampler2D effectTexture;

void main(){
	vec4 clr = texture(effectTexture,inUV);
	outFragColor = color * clr;
}