#include "effect.h"

//Global Effect variables
ID3DXMesh* billboardMesh = NULL;
D3DMATERIAL9 whiteMtrl;
SHADER effectVertexShader, effectPixelShader;
D3DXHANDLE effectMatW, effectMatVP, effectVCol;

//Global Effect Textures
IDirect3DTexture9* runesTexture = NULL;
IDirect3DTexture9* cloudTexture = NULL;

struct SimpleVertex
{
	SimpleVertex() {}
	SimpleVertex(D3DXVECTOR3 pos, D3DXVECTOR3 norm, D3DXVECTOR2 _uv)
	{
		position = pos;
		normal = norm;
		uv = _uv;
	}

	D3DXVECTOR3 position, normal;
	D3DXVECTOR2 uv;

	static const DWORD FVF;
};

const DWORD SimpleVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

void LoadEffectResources(IDirect3DDevice9* m_pDevice)
{
	//Calculate sight mesh (a simple quad)
	D3DXCreateMeshFVF(2, 4, D3DXMESH_MANAGED, SimpleVertex::FVF, m_pDevice, &billboardMesh);

	//Create 4 vertices
	SimpleVertex* v = 0;
	billboardMesh->LockVertexBuffer(0, (void**)&v);
	v[0] = SimpleVertex(D3DXVECTOR3(-0.5f, 0.0f, 0.5f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(0, 0));
	v[1] = SimpleVertex(D3DXVECTOR3(0.5f, 0.0f, 0.5f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(1, 0));
	v[2] = SimpleVertex(D3DXVECTOR3(-0.5f, 0.0f, -0.5f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(0, 1));
	v[3] = SimpleVertex(D3DXVECTOR3(0.5f, 0.0f, -0.5f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(1, 1));
	billboardMesh->UnlockVertexBuffer();

	//Create 2 faces
	WORD* indices = 0;
	billboardMesh->LockIndexBuffer(0, (void**)&indices);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 1; indices[4] = 3; indices[5] = 2;
	billboardMesh->UnlockIndexBuffer();

	//Set Attributes for the 2 faces
	DWORD* att = 0;
	billboardMesh->LockAttributeBuffer(0, &att);
	att[0] = 0; att[1] = 0;
	billboardMesh->UnlockAttributeBuffer();

	//Sight MTRL
	memset(&whiteMtrl, 0, sizeof(D3DMATERIAL9));
	whiteMtrl.Diffuse = whiteMtrl.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	//Load Shaders
	effectVertexShader.Init(m_pDevice, "shaders/effect.vs", VERTEX_SHADER);
	effectPixelShader.Init(m_pDevice, "shaders/effect.ps", PIXEL_SHADER);

	//Get constants
	effectMatW = effectVertexShader.GetConstant("matW");
	effectMatVP = effectVertexShader.GetConstant("matVP");
	effectVCol = effectVertexShader.GetConstant("vertexColor");

	//Load textures
	D3DXCreateTextureFromFile(m_pDevice, "textures/runes.dds", &runesTexture);
	D3DXCreateTextureFromFile(m_pDevice, "textures/cloud.dds", &cloudTexture);
}

void UnloadEffectResources()
{
	if (billboardMesh)billboardMesh->Release();
	billboardMesh = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								TRANSFORM													//
//////////////////////////////////////////////////////////////////////////////////////////////

TRANSFORM::TRANSFORM() { m_pos = m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f); m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f); }
TRANSFORM::TRANSFORM(D3DXVECTOR3 _pos) { m_pos = _pos; m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f); m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f); }
TRANSFORM::TRANSFORM(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot) { m_pos = _pos; m_rot = _rot; m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f); }
TRANSFORM::TRANSFORM(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot, D3DXVECTOR3 _sca) { m_pos = _pos; m_rot = _rot; m_sca = _sca; }

void TRANSFORM::Init(D3DXVECTOR3 _pos) { m_pos = _pos; m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f); m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f); }
void TRANSFORM::Init(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot) { m_pos = _pos; m_rot = _rot; m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f); }
void TRANSFORM::Init(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot, D3DXVECTOR3 _sca) { m_pos = _pos; m_rot = _rot; m_sca = _sca; }

D3DXMATRIX TRANSFORM::GetWorldMatrix()
{
	D3DXMATRIX p, r, s;
	D3DXMatrixTranslation(&p, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationYawPitchRoll(&r, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixScaling(&s, m_sca.x, m_sca.y, m_sca.z);
	D3DXMATRIX world = s * r * p;
	return world;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								EFFECTS	BASE CLASS											//
//////////////////////////////////////////////////////////////////////////////////////////////

EFFECT::EFFECT(IDirect3DDevice9* Dev)
{
	m_pDevice = Dev;
	m_time = 0.0f;
	m_color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);	//White
}

void EFFECT::PreRender()
{
	//Enable alpha blending
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);

	//Set vertex shader variables
	D3DXMATRIX view, proj;
	m_pDevice->GetTransform(D3DTS_VIEW, &view);
	m_pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	effectVertexShader.SetMatrix(effectMatVP, view * proj);
	effectVertexShader.SetVector4(effectVCol, m_color);

	//Set material
	m_pDevice->SetMaterial(&whiteMtrl);

	//enable Shaders
	effectVertexShader.Begin();
	effectPixelShader.Begin();
}

void EFFECT::PostRender()
{
	//Reset renderstates
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//Disable shaders
	effectVertexShader.End();
	effectPixelShader.End();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								EFFECT SPELL												//
//////////////////////////////////////////////////////////////////////////////////////////////

EFFECT_SPELL::EFFECT_SPELL(IDirect3DDevice9* Dev, D3DXVECTOR3 _pos) : EFFECT(Dev), m_t1(_pos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.1f, 0.1f, 0.1f))
{
	m_t1.m_pos.y += 1.0f;

	//Set a random m_color of the effect
	m_color = D3DXVECTOR4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 0.0f);

	//Initiate the glow transforms
	for (int i = 0; i < 10; i++)
	{
		float angle = i * (D3DX_PI / 5.0f);

		m_c[i].Init(_pos + D3DXVECTOR3(cos(angle) * 0.5f, 2.5f, sin(angle) * 0.5f),
			D3DXVECTOR3(D3DX_PI * 0.5f, angle, 0.0f),
			D3DXVECTOR3(4.0f, 4.0f, 6.0f));
	}
}

void EFFECT_SPELL::Update(float timeDelta)
{
	m_time += timeDelta;

	//Update Lower spinning quad...
	m_t1.m_rot.y += timeDelta;

	//Update cloud
	for (int i = 0; i < 10; i++)
		m_c[i].m_rot.y -= timeDelta;

	//Update Spinning quad scale
	if (m_time < 1.5f)
		m_t1.m_sca += D3DXVECTOR3(timeDelta, timeDelta, timeDelta) * 4.0f;
	if (m_time > 4.5f)
		m_t1.m_sca -= D3DXVECTOR3(timeDelta, timeDelta, timeDelta) * 4.0f;

	//Calculate alpha
	m_color.w = m_t1.m_sca.x / 6.0f;
}

void EFFECT_SPELL::Render()
{
	PreRender();

	if (billboardMesh)
	{
		//Spinning quad
		m_pDevice->SetTexture(0, runesTexture);
		effectVertexShader.SetMatrix(effectMatW, m_t1.GetWorldMatrix());
		billboardMesh->DrawSubset(0);

		//Glow
		m_pDevice->SetTexture(0, cloudTexture);
		for (int i = 0; i < 10; i++)
		{
			effectVertexShader.SetMatrix(effectMatW, m_c[i].GetWorldMatrix());
			billboardMesh->DrawSubset(0);
		}
	}

	PostRender();
}

bool EFFECT_SPELL::isDead()
{
	return m_t1.m_sca.x < 0.0f;
}