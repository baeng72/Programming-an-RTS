#type vertex
#version 460 core

layout(location=0) out vec4 aPos;
layout(location=1) out vec4 aColor;

struct PARTICLE_VERTEX{
	vec4 position;
	vec4 color;
};



readonly buffer PARTICLE_BUFFER{
	PARTICLE_VERTEX data[];
}particles;

void main(){
	//pass point to geometry shader
	aPos = particles.data[gl_VertexID].position;
	aColor = particles.data[gl_VertexID].color;
}

#type geometry
#version 460 core
layout(points) in;
layout(triangle_strip,max_vertices=4) out;

layout(location=0) in vec4 aPos[];
layout(location=1) in vec4 aColor[];

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outUV;

uniform	mat4 viewProj;
uniform	vec3 eyePosW;
uniform	vec2 sizeW;	


//vec2 uvs[] = {vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,1)};
vec2 uvs[] = {vec2(1,1),vec2(1,0),vec2(0,1),vec2(0,0)};

void main(){
//
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	//
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	
	vec3 look = eyePosW - vec3(aPos[0]);
	look.y = 0.0f; // y-axis aligned, so project to xz-plane
	look = normalize(look);
	vec3 right = cross(up, look);
	
	//
	// Compute triangle strip vertices (quad) in world space.
	//
	float halfWidth  = 0.5f*sizeW.x;
	float halfHeight = 0.5f*sizeW.y;
	
	vec4 v[4];
	v[0] = vec4(vec3(aPos[0]) + halfWidth*right - halfHeight*up, 1.0f);
	v[1] = vec4(vec3(aPos[0]) + halfWidth*right + halfHeight*up, 1.0f);
	v[2] = vec4(vec3(aPos[0]) - halfWidth*right - halfHeight*up, 1.0f);
	v[3] = vec4(vec3(aPos[0]) - halfWidth*right + halfHeight*up, 1.0f);
	
	
	//
	// Transform quad vertices to world space and output 
	// them as a triangle strip.
	//
	for(int i=0;i<4;++i){
		gl_Layer=0;//one face?		
		gl_Position = viewProj*v[i];				
		outColor = aColor[0];
		outUV = uvs[i];
		EmitVertex();
	}
	EndPrimitive();    
}

#type fragment
#version 450

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inUV;
layout(location=0) out vec4 outFragColor;

uniform sampler2D texmap;

void main(){
	vec4 clr = texture(texmap,inUV);
	outFragColor = clr * inColor;
}