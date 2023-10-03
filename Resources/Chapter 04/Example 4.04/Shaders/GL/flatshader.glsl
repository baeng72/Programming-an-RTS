#type vertex
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;

out vec3 aNormal;
out vec4 aColor;

uniform mat4 viewProj;
uniform	mat4 model;

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	aNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	aColor = inColor;
}

#type fragment
#version 460 core
layout(location=0) in vec3 aNormal;
layout(location=1) in vec4 aColor;

out vec4 outFragColor;

void main(){
	outFragColor = aColor;
}