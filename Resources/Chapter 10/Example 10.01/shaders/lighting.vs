//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                    Unit Lighting Vertexshader                        //
//                                                                      //
//                   Written by C. Granberg, 2006                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

uniform extern float4x4 matW;
uniform extern float4x4 matVP;
uniform extern float3 DirToSun;
uniform extern float4 vertexCol;
uniform extern float3 mapSize;

struct VS_INPUT
{
   float4 position : POSITION0;
   float3 normal : NORMAL0;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 position : POSITION0;
   float4 color : COLOR;
   float2 uv : TEXCOORD0;
   float2 uv2 : TEXCOORD1;
   float  shade : TEXCOORD2;
};

VS_OUTPUT Main(VS_INPUT input)
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   //Transform vertex to world space
   float4 worldPos = mul(input.position, matW);

   //Transform vertex to screen space
   output.position = mul(worldPos, matVP);
   
   //Transform normal
   float3 normal = normalize(mul(input.normal, matW));

   // Do basic diffuse lighting calculating to compute vertex shade.
   output.shade = max(0.0f, dot(normal, DirToSun));
   output.shade = 0.2f + output.shade * 0.8f;
  
   output.color = vertexCol;
   output.uv = input.uv;

   //Calculate the Terrain-UV coordinate
   output.uv2 = float2(worldPos.x / mapSize.x, -worldPos.z / mapSize.y);

   return output;
}

