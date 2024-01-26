uniform extern float4x4 matW;
uniform extern float4x4 matVP;
uniform extern float3 DirToSun;
uniform extern float3 mapSize;

struct VS_INPUT1
{
   float4 position : POSITION0;
   float3 normal : NORMAL0;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT1
{
   float4 position : POSITION0;
   float2 uv : TEXCOORD0;
   float2 uv2 : TEXCOORD1;
   float  shade : TEXCOORD2;
};

VS_OUTPUT1 Main(VS_INPUT1 input)
{
   VS_OUTPUT1 output = (VS_OUTPUT1)0;

   //transform World, View and Projection
   float4 temp = mul(input.position, matW);
   output.position = mul(temp, matVP);
   input.normal = mul(input.normal, matW);

   //Directional Lighting
   output.shade = max(0.0f, dot(normalize(input.normal), DirToSun));
   output.shade = 0.2f + output.shade * 0.8f;

   //Set UV coordinates
   output.uv = input.uv;
   output.uv2 = float2(temp.x / mapSize.x, -temp.z / mapSize.y);

   return output;
}

