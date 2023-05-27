#include "effect.h"

//Global Effect variables
ID3DXMesh* billboardMesh = NULL;
D3DMATERIAL9 whiteMtrl;
SHADER effectVertexShader, effectPixelShader;
D3DXHANDLE effectMatW, effectMatVP, effectVCol;
ID3DXSprite* sprite = NULL;

//Global Effect Textures
IDirect3DTexture9* runesTexture = NULL;
IDirect3DTexture9* cloudTexture = NULL;
IDirect3DTexture9* fireballTexture = NULL;
IDirect3DTexture9* lensflareTexture = NULL;

struct SimpleVertex
{
	SimpleVertex() {}
	SimpleVertex(D3DXVECTOR3 pos, D3DXVECTOR3 norm, D3DXVECTOR2 _uv)
	{
		m_position = pos;
		normal = norm;
		uv = _uv;
	}

	D3DXVECTOR3 m_position, normal;
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

	//Create Sprite
	D3DXCreateSprite(m_pDevice, &sprite);

	//Load textures
	D3DXCreateTextureFromFile(m_pDevice, "textures/runes.dds", &runesTexture);
	D3DXCreateTextureFromFile(m_pDevice, "textures/cloud.dds", &cloudTexture);
	D3DXCreateTextureFromFile(m_pDevice, "textures/fireball.dds", &fireballTexture);
	D3DXCreateTextureFromFile(m_pDevice, "textures/lensflare.dds", &lensflareTexture);
}

void UnloadEffectResources()
{
	if (billboardMesh)billboardMesh->Release();
	billboardMesh = NULL;

	if (sprite)sprite->Release();
	sprite = NULL;

	//Release textures
	if (runesTexture)runesTexture->Release();
	if (cloudTexture)cloudTexture->Release();
	if (fireballTexture)fireballTexture->Release();
	if (lensflareTexture)lensflareTexture->Release();

	runesTexture = NULL;
	cloudTexture = NULL;
	fireballTexture = NULL;
	lensflareTexture = NULL;
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
//								EFFECTS	SPELL												//
//////////////////////////////////////////////////////////////////////////////////////////////

EFFECT_SPELL::EFFECT_SPELL(IDirect3DDevice9* Dev, D3DXVECTOR3 _pos) : EFFECT(Dev), m_t1(_pos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.1f, 0.1f, 0.1f))
{
	m_t1.m_pos.y += 1.0f;
	m_color = D3DXVECTOR4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 0.0f);

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
	if (m_time < 1.5f)m_t1.m_sca += D3DXVECTOR3(timeDelta, timeDelta, timeDelta) * 4.0f;
	if (m_time > 4.5f)m_t1.m_sca -= D3DXVECTOR3(timeDelta, timeDelta, timeDelta) * 4.0f;

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

		//Cloud
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

//////////////////////////////////////////////////////////////////////////////////////////////
//								EFFECTS	FIREBALL											//
//////////////////////////////////////////////////////////////////////////////////////////////

EFFECT_FIREBALL::EFFECT_FIREBALL(IDirect3DDevice9* Dev, BONE* _src, D3DXVECTOR3 _dest) : EFFECT(Dev)
{
	m_pSrcBone = _src;
	dest = _dest;
	m_color.w = 0.01f;
	m_prc = 0.0f;
	m_speed = 22.0f;

	if (m_pSrcBone != NULL)
	{
		D3DXMATRIX mat = m_pSrcBone->CombinedTransformationMatrix;
		m_t1.Init(D3DXVECTOR3(mat(3, 0), mat(3, 1), mat(3, 2)), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.1f, 0.1f, 0.1f));
	}
}

void EFFECT_FIREBALL::Update(float timeDelta)
{
	m_t1.m_rot += D3DXVECTOR3(0.5f, 0.5f, 0.5f) * timeDelta;

	if (m_time < 1.0f && m_pSrcBone != NULL)		//Follow staff
	{
		m_time += timeDelta;
		D3DXMATRIX mat = m_pSrcBone->CombinedTransformationMatrix;
		m_t1.m_pos = D3DXVECTOR3(mat(3, 0), mat(3, 1), mat(3, 2));		//Extract position from bone
		m_t1.m_sca = D3DXVECTOR3(1.5f, 1.5f, 1.5f) * m_time;
		m_color.w = m_time;

		if (m_time > 1.0f)
		{
			m_color.w = m_time * 0.5f;
			origin = m_t1.m_pos;
			D3DXVECTOR3 vec = (origin - dest);
			m_length = D3DXVec3Length(&vec);
		}
	}
	else if (m_prc < 1.0f)		//Fly towards target
	{
		m_prc += (m_speed * timeDelta) / m_length;
		m_t1.m_pos = GetPosition(m_prc);
	}
	else					//Explode
	{
		m_prc += (m_speed * timeDelta) / m_length;
		m_t1.m_sca += D3DXVECTOR3(5.0f, 5.0f, 5.0f) * timeDelta;
		m_color.w -= timeDelta * 0.5f;
	}
}

void EFFECT_FIREBALL::Render()
{
	PreRender();

	if (billboardMesh)
	{
		D3DXVECTOR3 orgRot = m_t1.m_rot;
		D3DXVECTOR3 rotations[] = { D3DXVECTOR3(0.0f, 0.0f, 0.0f),
								   D3DXVECTOR3(D3DX_PI * 0.5f, 0.0f, 0.0f),
								   D3DXVECTOR3(0.0f, D3DX_PI * 0.5f, 0.0f),
								   D3DXVECTOR3(0.0f, 0.0f, D3DX_PI * 0.5f) };

		D3DXVECTOR3 orgPos = m_t1.m_pos;
		D3DXVECTOR3 positions[] = { m_t1.m_pos,
								   GetPosition(m_prc - (1.5f / m_length)),
								   GetPosition(m_prc - (2.5f / m_length)),
								   GetPosition(m_prc - (3.25f / m_length)),
								   GetPosition(m_prc - (4.0f / m_length)) };

		D3DXVECTOR3 orgSca = m_t1.m_sca;
		D3DXVECTOR3 scales[] = { m_t1.m_sca,
								m_t1.m_sca * 0.8f,
								m_t1.m_sca * 0.6f,
								m_t1.m_sca * 0.4f,
								m_t1.m_sca * 0.2f };

		m_pDevice->SetTexture(0, fireballTexture);
		for (int t = 0; t < 5; t++)
			for (int i = 0; i < 4; i++)
			{
				m_t1.m_pos = positions[t];
				m_t1.m_rot = orgRot + rotations[i];
				m_t1.m_sca = scales[t];

				effectVertexShader.SetMatrix(effectMatW, m_t1.GetWorldMatrix());
				billboardMesh->DrawSubset(0);
			}

		m_t1.m_pos = orgPos;
		m_t1.m_rot = orgRot;
		m_t1.m_sca = orgSca;
	}

	PostRender();
}

bool EFFECT_FIREBALL::isDead()
{
	return m_pSrcBone == NULL || m_color.w < 0.0f;
}

D3DXVECTOR3 EFFECT_FIREBALL::GetPosition(float p)
{
	if (p < 0.0f)p = 0.0f;
	if (p > 1.0f)p = 1.0f;

	D3DXVECTOR3 m_pos = origin * (1.0f - p) + dest * p;	//Lerp between origin and dest
	m_pos.y += sin(p * D3DX_PI) * 3.0f;						//Add Arc
	return m_pos;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								EFFECTS	LENSFLARE											//
//////////////////////////////////////////////////////////////////////////////////////////////

EFFECT_LENSFLARE::EFFECT_LENSFLARE(IDirect3DDevice9* Dev, int _type, D3DXVECTOR3 _position) : EFFECT(Dev)
{
	m_position = _position;
	m_type = _type;
	m_mainAlpha = 0.0f;
	m_inScreen = false;

	//Add Flares
	if (m_type == 0)	//Standard flare
	{
		m_flares.push_back(FLARE(D3DXCOLOR(1.0f, 1.0f, 0.5f, 1.0f), 0.5f, 0.7f, 0));
		m_flares.push_back(FLARE(D3DXCOLOR(0.0f, 1.0f, 0.5f, 1.0f), 1.0f, 1.0f, 1));
		m_flares.push_back(FLARE(D3DXCOLOR(1.0f, 0.5f, 0.5f, 1.0f), 1.5f, 1.3f, 2));
		m_flares.push_back(FLARE(D3DXCOLOR(1.0f, 1.0f, 0.5f, 1.0f), -0.5f, 0.8f, 3));
		m_flares.push_back(FLARE(D3DXCOLOR(0.0f, 1.0f, 0.5f, 1.0f), 0.4f, 1.0f, 4));
		m_flares.push_back(FLARE(D3DXCOLOR(1.0f, 1.0f, 0.5f, 1.0f), 0.75f, 1.0f, 5));
		m_flares.push_back(FLARE(D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f), 1.8f, 1.2f, 6));
		m_flares.push_back(FLARE(D3DXCOLOR(1.0f, 1.0f, 0.5f, 1.0f), 2.1f, 0.5f, 4));
	}
	else if (m_type == 1)		//Some other flare etc...
	{
		//setup your own flare here...
	}
}

void EFFECT_LENSFLARE::Update(float timeDelta)
{
	if (m_inScreen)
		m_mainAlpha += timeDelta * 3.0f;
	else m_mainAlpha -= timeDelta * 3.0f;

	if (m_mainAlpha > 1.0f)m_mainAlpha = 1.0f;
	if (m_mainAlpha < 0.0f)m_mainAlpha = 0.0f;
}

void EFFECT_LENSFLARE::Render()
{
	if (sprite == NULL || lensflareTexture == NULL)return;

	RECT sourceRectangles[7] = { {0, 0, 128, 128},
								{128, 0, 256, 128},
								{0, 128, 128, 256},
								{128, 128, 192, 192},
								{192, 128, 256, 192},
								{128, 192, 192, 256},
								{192, 192, 256, 256} };

	//Calculate screen position of light source
	D3DXVECTOR3 screenPos;
	D3DVIEWPORT9 Viewport;
	D3DXMATRIX Projection, View, World;
	m_pDevice->GetViewport(&Viewport);
	m_pDevice->GetTransform(D3DTS_VIEW, &View);
	m_pDevice->GetTransform(D3DTS_PROJECTION, &Projection);
	D3DXMatrixIdentity(&World);
	D3DXVec3Project(&screenPos, &m_position, &Viewport, &Projection, &View, &World);

	//Get viewport
	D3DVIEWPORT9 v;
	m_pDevice->GetViewport(&v);

	//Check that light source is within or without the screen bounds
	if (screenPos.x < 0 || screenPos.x > v.Width ||
		screenPos.y < 0 || screenPos.y > v.Height || screenPos.z > 1.0f)
		m_inScreen = false;
	else m_inScreen = true;

	//Lensflares aren't visible so exit function...
	if (m_mainAlpha <= 0.0f)return;

	//Calculate the ray from the screen center to the light source
	D3DXVECTOR2 lightSource = D3DXVECTOR2(screenPos.x, screenPos.y);
	D3DXVECTOR2 screenCenter = D3DXVECTOR2(v.Width * 0.5f, v.Height * 0.5f);
	D3DXVECTOR2 ray = screenCenter - lightSource;

	//Draw the different flares
	D3DXMATRIX sca;
	sprite->Begin(D3DXSPRITE_ALPHABLEND);
	for (int i = 0; i < m_flares.size(); i++)
	{
		//Calculate Flare position in screen coordinates
		RECT r = sourceRectangles[m_flares[i].sourceFlare];
		D3DXVECTOR2 offset = D3DXVECTOR2((r.right - r.left) / 2.0f, (r.bottom - r.top) / 2.0f) * m_flares[i].scale;
		D3DXVECTOR2 flarePos = lightSource + ray * m_flares[i].place - offset;

		//Scale
		D3DXMatrixScaling(&sca, m_flares[i].scale, m_flares[i].scale, 1.0f);

		//Calculate flare alpha
		D3DXCOLOR m_color = m_flares[i].m_color;
		D3DXVECTOR2 vec2 = ((flarePos + offset) - screenCenter);
		float alpha = (D3DXVec2Length(&vec2) + 150.0f) / (float)v.Height;
		if (alpha > 1.0f)alpha = 1.0f;
		m_color.a = alpha * m_mainAlpha * 1.5f;

		//Draw Flare
		sprite->SetTransform(&sca);
		D3DXVECTOR3 vec = D3DXVECTOR3(flarePos.x / m_flares[i].scale, flarePos.y / m_flares[i].scale, 0.0f);
		sprite->Draw(lensflareTexture, &r, NULL, &vec, m_color);
	}
	sprite->End();

	D3DXMatrixIdentity(&sca);
	sprite->SetTransform(&sca);
}

bool EFFECT_LENSFLARE::isDead()
{
	return false;
}