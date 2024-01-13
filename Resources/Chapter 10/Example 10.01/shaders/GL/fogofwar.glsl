#type vertex
#version 460 core

layout (location=0) out vec2 outUV;

vec2 positions[6] = vec2[](
	vec2(-1.0,-1.0),
	vec2(1.0,-1.0),
	vec2(-1.0,1.0),
	vec2(-1.0,1.0),
	vec2(1.0,-1.0),
	vec2(1.0,1.0)	
);

vec2 uvs[6] = vec2[](
	vec2(0.0,0.0),
	vec2(1.0,0.0),
	vec2(0.0,1.0),
	vec2(0.0,1.0),
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

layout (binding=0) uniform sampler2D a_visibleTexture;
layout (binding=1) uniform sampler2D b_visitedTexture;
layout (binding=2) uniform sampler2D c_lightMap;


void main(){		
	vec4 visibleAlpha = texture(a_visibleTexture,inUV);
	vec4 visitedAlpha =  texture(b_visitedTexture,vec2(inUV.x,1-inUV.y))*0.75;
	vec4 light = texture(c_lightMap,inUV);
	
	outFragColor = vec4(max(visibleAlpha.rgb,visitedAlpha.rgb),1.0)*light.r;	
}