//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                        Effect Vertexshader                           //
//                                                                      //
//                   Written by C. Granberg, 2006                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

uniform extern float4x4 matW;
uniform extern float4x4 matVP;
uniform extern float4 vertexColor;

struct VS_INPUT
{
   float4 position : POSITION0;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 position : POSITION0;
   float4 color : COLOR0;
   float2 uv : TEXCOORD0;
};

VS_OUTPUT Main(VS_INPUT input)
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   // Project position homogeneous clip space.
   float4 temp = mul(input.position, matW);
   output.position = mul(temp, matVP);
   output.uv = input.uv;
   output.color = vertexColor;

   return output;
}

