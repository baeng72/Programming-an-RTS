#type vertex
#version 450

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
	
	gl_Position = vec4(positions[gl_VertexIndex],0.0,1.0);
	outUV = uvs[gl_VertexIndex];
}

#type fragment
#version 450

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout (binding=0) uniform sampler2D visibleTexture;
layout (binding=1) uniform sampler2D visitedTexture;
layout (binding=2) uniform sampler2D lightMap;


void main(){		
	vec4 visibleAlpha = texture(visibleTexture,inUV);
	vec4 visitedAlpha =  texture(visitedTexture,inUV)*0.75;
	vec4 light = texture(lightMap,inUV);
	
	outFragColor = vec4(max(visibleAlpha.rgb,visitedAlpha.rgb),1.0)*light.r;	
}