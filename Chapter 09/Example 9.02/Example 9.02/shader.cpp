#include "shader.h"

SHADER::SHADER()
{
	PixelShader = NULL;
	VertexShader = NULL;
	ConstantTable = NULL;
}

SHADER::~SHADER()
{
	if (PixelShader != NULL)
	{
		PixelShader->Release();
		PixelShader = NULL;
	}

	if (VertexShader != NULL)
	{
		VertexShader->Release();
		VertexShader = NULL;
	}
}

void SHADER::Init(IDirect3DDevice9* Dev, const char fName[], int typ)
{
	Device = Dev;
	type = typ;
	if (Device == NULL)return;

	// Assemble and set the pixel or vertex shader 	
	HRESULT hRes;
	LPD3DXBUFFER Code = NULL;
	LPD3DXBUFFER ErrorMsgs = NULL;

	if (type == PIXEL_SHADER)
		hRes = D3DXCompileShaderFromFile(fName, NULL, NULL, "Main", "ps_2_0", D3DXSHADER_DEBUG, &Code, &ErrorMsgs, &ConstantTable);
	else hRes = D3DXCompileShaderFromFile(fName, NULL, NULL, "Main", "vs_2_0", D3DXSHADER_DEBUG, &Code, &ErrorMsgs, &ConstantTable);

	if ((FAILED(hRes)) && (ErrorMsgs != NULL))		//If failed
	{
		if (type == PIXEL_SHADER)
			debug.Print("Couldnt compile the Pixel Shader");
		else debug.Print("Couldnt compile the Vertex Shader");

		debug.Print((char*)ErrorMsgs->GetBufferPointer());
		return;
	}

	if (type == PIXEL_SHADER)
		hRes = Device->CreatePixelShader((DWORD*)Code->GetBufferPointer(), &PixelShader);
	else hRes = Device->CreateVertexShader((DWORD*)Code->GetBufferPointer(), &VertexShader);

	if (FAILED(hRes))
	{
		if (type == PIXEL_SHADER)
			debug.Print("Couldnt Create the Pixel Shader");
		else debug.Print("Couldnt Create the Vertex Shader");
		exit(0);
	}
}

void SHADER::Begin()
{
	if (type == PIXEL_SHADER)
		HRESULT hRes = Device->SetPixelShader(PixelShader);
	else HRESULT hRes = Device->SetVertexShader(VertexShader);
}

void SHADER::End()
{
	if (type == PIXEL_SHADER)
		HRESULT hRes = Device->SetPixelShader(NULL);
	else HRESULT hRes = Device->SetVertexShader(NULL);
}

D3DXHANDLE SHADER::GetConstant(const char name[])
{
	return ConstantTable->GetConstantByName(NULL, name);
}

void SHADER::SetFloat(D3DXHANDLE h, float f)
{
	ConstantTable->SetFloat(Device, h, f);
}

void SHADER::SetVector3(D3DXHANDLE h, D3DXVECTOR3 v)
{
	ConstantTable->SetValue(Device, h, &v, sizeof(D3DXVECTOR3));
}

void SHADER::SetVector4(D3DXHANDLE h, D3DXVECTOR4 v)
{
	ConstantTable->SetVector(Device, h, &v);
}

void SHADER::SetMatrix(D3DXHANDLE h, D3DXMATRIX m)
{
	ConstantTable->SetMatrix(Device, h, &m);
}