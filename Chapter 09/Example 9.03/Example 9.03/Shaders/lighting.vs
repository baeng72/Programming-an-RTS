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
   float  shade : TEXCOORD1;
};

VS_OUTPUT Main(VS_INPUT input)
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   // Project position homogeneous clip space.
   float4 temp = mul(input.position, matW);
   output.position = mul(temp, matVP);

   // Do basic diffuse lighting calculating to compute vertex shade.
   output.shade = max(0.0f, dot(normalize(input.normal), DirToSun));
   output.shade = 0.2f + output.shade * 0.8f;
  
   output.color = vertexCol;
   output.uv = input.uv;

   return output;
}

