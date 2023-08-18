#type vertex
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec4 outColor;

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;
};

layout (push_constant) uniform PushConst{
	mat4 model;
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outColor = inColor;
}

#type fragment
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec4 inColor;

layout(location=0) out vec4 outFragColor;

void main(){
	outFragColor = inColor;
}