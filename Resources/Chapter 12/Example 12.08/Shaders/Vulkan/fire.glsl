#type vertex
#version 460
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNorm;
layout (location=2) in vec2 inUV;

layout (location=0) out vec2 aUV;
layout (location=1) out vec2 aUV_Noise1;
layout (location=2) out vec2 aUV_Noise2;
layout (location=3) out vec2 aUV_Noise3;
layout (location=4) out float aMainAlpha;

layout(binding=0) uniform UBO{
	mat4 matVP;
	vec3 DirToCam;
};
layout(push_constant) uniform PushConst{
	mat4 matWorld;
	float time;
	float offset;
};



void main(){
	gl_Position = matVP * matWorld * vec4(inPos,1.0);
	aUV = inUV;
	//Generate noise coordinates as a function of time
   aUV_Noise1 = inUV + vec2(time * 0.7f + offset, time);
   aUV_Noise2 = inUV + vec2(-time * 0.11f, time * 0.93f + offset);
   aUV_Noise3 = inUV + vec2(time * 0.3f - offset, time * 0.71f);
   vec3 norm = vec3(matWorld * vec4(inNorm,0.0));
   aMainAlpha = abs(dot(normalize(norm), DirToCam));
}

#type fragment
#version 460
layout(location=0) in vec2 aUV;
layout (location=1) in vec2 aUV_Noise1;
layout (location=2) in vec2 aUV_Noise2;
layout (location=3) in vec2 aUV_Noise3;
layout (location=4) in float aMainAlpha;
layout(location=0) out vec4 outFragColor;


layout(binding=1) uniform sampler2D fireTexture;
layout(binding=2) uniform sampler2D noiseTexture;

void main(){
	//Sample noise pixels
    float n1 = texture(noiseTexture, aUV_Noise1).r;
    float n2 = texture(noiseTexture, aUV_Noise2).r;
    float n3 = texture(noiseTexture, aUV_Noise3).r;
	//Sum up noise and perturb the x coordinate of the main UV
    vec2 uv = aUV;
	uv.x += (n1 + n2 + n3) * 0.1f - 0.05f;
	vec4 clr = texture(fireTexture,uv);
	outFragColor = clr * aMainAlpha;
}