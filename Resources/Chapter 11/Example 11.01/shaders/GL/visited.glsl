#type vertex
#version 460 core

layout (location=0) out vec2 outUV;


vec2 positions[6] = vec2[](
	vec2(-1.0,-1.0),
	vec2(1.0,-1.0),
	vec2(1.0,1.0),
	vec2(-1.0,-1.0),
	vec2(1.0,1.0),
	vec2(-1.0,1.0)	
);

vec2 uvs[6] = vec2[](
	vec2(0.0,1-0.0),
	vec2(1.0,1-0.0),
	vec2(1.0,1-1.0),
	vec2(0.0,1-0.0),
	vec2(1.0,1-1.0),
	vec2(0.0,1-1.0)
);

void main(){			
	gl_Position = vec4(positions[gl_VertexID],0.5,1.0);
	outUV = uvs[gl_VertexID];	
}

#type fragment
#version 460 core

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

uniform sampler2D visibleTexture;
uniform sampler2D visitedTexture;


void main(){		
	vec3 visibleColor = texture(visibleTexture,inUV).xyz;
	vec3 visitedColor =  texture(visitedTexture,inUV).xyz;
	vec3 color = max(visibleColor,visitedColor);	
	outFragColor = vec4(color,1.0);
}