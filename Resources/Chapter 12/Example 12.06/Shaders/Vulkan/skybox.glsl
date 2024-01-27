#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUV;

layout(location=0) out vec2 outUV;


layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
};

layout(push_constant) uniform PushConst{
	mat4 world;
};

void main(){
	gl_Position = viewProj * world * vec4(inPos,1.0);
	outUV = inUV;
}

#type fragment
#version 450
layout(location=0) in vec2 inUV;
layout(location=0) out vec4 outFragColor;

layout(set=0,binding=1) uniform sampler2D texmap;


void main(){
	vec4 clr = texture(texmap,inUV);
	outFragColor=clr;	
}