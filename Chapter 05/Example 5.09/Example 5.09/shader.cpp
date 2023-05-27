#include "shader.h"

SHADER::SHADER()
{
	m_pPixelShader = NULL;
	m_pVertexShader = NULL;
	m_pConstantTable = NULL;
}

SHADER::~SHADER()
{
	if (m_pPixelShader != NULL)
	{
		m_pPixelShader->Release();
		m_pPixelShader = NULL;
	}

	if (m_pVertexShader != NULL)
	{
		m_pVertexShader->Release();
		m_pVertexShader = NULL;
	}
}

void SHADER::Init(IDirect3DDevice9* Dev, const char fName[], int typ)
{
	m_pDevice = Dev;
	m_type = typ;
	if (m_pDevice == NULL)return;

	// Assemble and set the pixel or vertex shader 	
	HRESULT hRes;
	LPD3DXBUFFER Code = NULL;
	LPD3DXBUFFER ErrorMsgs = NULL;

	if (m_type == PIXEL_SHADER)
		hRes = D3DXCompileShaderFromFile(fName, NULL, NULL, "Main", "ps_2_0", D3DXSHADER_DEBUG, &Code, &ErrorMsgs, &m_pConstantTable);
	else hRes = D3DXCompileShaderFromFile(fName, NULL, NULL, "Main", "vs_2_0", D3DXSHADER_DEBUG, &Code, &ErrorMsgs, &m_pConstantTable);

	if ((FAILED(hRes)) && (ErrorMsgs != NULL))		//If failed
	{
		if (m_type == PIXEL_SHADER)
			debug.Print("Couldnt compile the Pixel Shader");
		else debug.Print("Couldnt compile the Vertex Shader");

		debug.Print((char*)ErrorMsgs->GetBufferPointer());
		return;
	}

	if (m_type == PIXEL_SHADER)
		hRes = m_pDevice->CreatePixelShader((DWORD*)Code->GetBufferPointer(), &m_pPixelShader);
	else hRes = m_pDevice->CreateVertexShader((DWORD*)Code->GetBufferPointer(), &m_pVertexShader);

	if (FAILED(hRes))
	{
		if (m_type == PIXEL_SHADER)
			debug.Print("Couldnt Create the Pixel Shader");
		else debug.Print("Couldnt Create the Vertex Shader");
		exit(0);
	}
}

void SHADER::Begin()
{
	if (m_type == PIXEL_SHADER)
		HRESULT hRes = m_pDevice->SetPixelShader(m_pPixelShader);
	else HRESULT hRes = m_pDevice->SetVertexShader(m_pVertexShader);
}

void SHADER::End()
{
	if (m_type == PIXEL_SHADER)
		HRESULT hRes = m_pDevice->SetPixelShader(NULL);
	else HRESULT hRes = m_pDevice->SetVertexShader(NULL);
}

D3DXHANDLE SHADER::GetConstant(char name[])
{
	return m_pConstantTable->GetConstantByName(NULL, name);
}

void SHADER::SetFloat(D3DXHANDLE h, float f)
{
	m_pConstantTable->SetFloat(m_pDevice, h, f);
}

void SHADER::SetVector3(D3DXHANDLE h, D3DXVECTOR3 v)
{
	m_pConstantTable->SetValue(m_pDevice, h, &v, sizeof(D3DXVECTOR3));
}

void SHADER::SetVector4(D3DXHANDLE h, D3DXVECTOR4 v)
{
	m_pConstantTable->SetVector(m_pDevice, h, &v);
}

void SHADER::SetMatrix(D3DXHANDLE h, D3DXMATRIX m)
{
	m_pConstantTable->SetMatrix(m_pDevice, h, &m);
}