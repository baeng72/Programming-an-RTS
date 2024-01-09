#type vertex
#version 460 core

layout (location=0) out vec2 outUV;


vec2 positions[6] = vec2[](
	vec2(-1.0,1.0),
	vec2(1.0,1.0),
	vec2(-1.0,-1.0),
	vec2(-1.0,-1.0),
	vec2(1.0,1.0),
	vec2(1.0,-1.0)	
);

vec2 uvs[6] = vec2[](
	vec2(0.0,1.0),
	vec2(1.0,1.0),
	vec2(0.0,0.0),
	vec2(0.0,0.0),
	vec2(1.0,0.0),
	vec2(1.0,1.0)
);

void main(){		
	gl_Position = vec4(positions[gl_VertexID],0.0,1.0);
	outUV = uvs[gl_VertexID];	
}

#type fragment
#version 460 core

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

uniform sampler2D landscapeTexture;
uniform sampler2D fogOfWarTexture;


void main(){		
	vec3 visibleColor = texture(landscapeTexture,inUV).xyz;
	vec3 visitedColor =  texture(fogOfWarTexture,inUV).xyz;
	vec3 color = visibleColor * visitedColor;	
	outFragColor = vec4(color,1.0);
}