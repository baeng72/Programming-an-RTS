//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                   Terrain Lighting Vertexshader                      //
//                                                                      //
//                   Written by C. Granberg, 2006                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

uniform extern float4x4 matW;
uniform extern float4x4 matVP;
uniform extern float3 DirToSun;

struct VS_INPUT
{
   float4 position : POSITION0;
   float3 normal : NORMAL0;
   float2 uv : TEXCOORD0;         //alpha UV
   float2 uv2 : TEXCOORD1;        //diffuse UV
};

struct VS_OUTPUT
{
   float4 position : POSITION0;
   float2 uv : TEXCOORD0;
   float2 uv2 : TEXCOORD1;
   float  shade : TEXCOORD2;
};

VS_OUTPUT Main(VS_INPUT input)
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   //transform World, View and Projection
   float4 temp = mul(input.position, matW);
   output.position = mul(temp, matVP);

   //Directional Lighting
   output.shade = max(0.0f, dot(normalize(input.normal), DirToSun));
   output.shade = 0.2f + output.shade * 0.8f;

   //Copy UV coordinates
   output.uv = input.uv;
   output.uv2 = input.uv2;

   return output;
}

