//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                        Fire Vertexshader                             //
//                                                                      //
//                   Written by C. Granberg, 2006                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

uniform extern float4x4 matW;
uniform extern float4x4 matVP;
uniform extern float3 DirToCam;
uniform extern float time;
uniform extern float offset;

struct VS_INPUT
{
   float4 position : POSITION0;
   float3 normal   : NORMAL0;
   float2 UV       : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 position  : POSITION0;
   float2 UV        : TEXCOORD0;
   float2 UV_Noise1 : TEXCOORD1;
   float2 UV_Noise2 : TEXCOORD2;
   float2 UV_Noise3 : TEXCOORD3;
   float  mainAlpha : TEXCOORD4;
};

VS_OUTPUT Main(VS_INPUT input)
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   // Project position
   float4 temp = mul(input.position, matW);
   output.position = mul(temp, matVP);

   //copy main UV coordinates across 
   output.UV = input.UV;

   //Generate noise coordinates as a function of time
   output.UV_Noise1 = input.UV + float2(time * 0.7f + offset, time);
   output.UV_Noise2 = input.UV + float2(-time * 0.11f, time * 0.93f + offset);
   output.UV_Noise3 = input.UV + float2(time * 0.3f - offset, time * 0.71f);

   //Transform normal
   input.normal = mul(input.normal, matW);

   //Calculate mainAlpha
   output.mainAlpha = abs(dot(normalize(input.normal), DirToCam));

   return output;
}

