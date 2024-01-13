#type vertex
#version 460 core

layout (location=0) out vec2 outUV;
layout (location=1) out vec2 outUV2;

vec2 positions[6] = vec2[](
	vec2(-1.0,-1.0),
	vec2(1.0,-1.0),
	vec2(1.0,1.0),
	vec2(-1.0,-1.0),
	vec2(1.0,1.0),
	vec2(-1.0,1.0)	
);

vec2 uvs[6] = vec2[](
	vec2(0.0,0.0),
	vec2(1.0,0.0),
	vec2(1.0,1.0),
	vec2(0.0,0.0),
	vec2(1.0,1.0),
	vec2(0.0,1.0)
);

void main(){		
	gl_Position = vec4(positions[gl_VertexID],0.5,1.0);
	outUV = uvs[gl_VertexID];
	outUV2 = vec2(uvs[gl_VertexID].x,1-uvs[gl_VertexID].y);
}

#type fragment
#version 460 core

layout (location=0) in vec2 inUV;
layout (location=1) in vec2 inUV2;
layout (location=0) out vec4 outFragColor;

uniform sampler2D a_visibleTexture;
uniform sampler2D b_visitedTexture;


void main(){		
	vec3 visibleColor = texture(a_visibleTexture,inUV).xyz;
	//vec3 visibleColor=vec3(0.0,0.0,0.0);
	vec3 visitedColor =  texture(b_visitedTexture,inUV2).xyz;
	vec3 color = max(visibleColor,visitedColor);	
	outFragColor = vec4(color,1.0);
}