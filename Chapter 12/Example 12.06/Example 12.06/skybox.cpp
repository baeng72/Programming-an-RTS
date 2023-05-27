#include "skybox.h"

struct SKYBOX_VERTEX
{
	SKYBOX_VERTEX(D3DXVECTOR3 _pos, D3DXVECTOR2 _uv) { pos = _pos; uv = _uv; }
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
	static const DWORD FVF;
};

const DWORD SKYBOX_VERTEX::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

SKYBOX::SKYBOX(IDirect3DDevice9* Dev, const char fileName[], float size)
{
	m_pDevice = Dev;
	std::string endings[] = { "_UP.jpg", "_FR.jpg", "_BK.jpg", "_RT.jpg", "_LF.jpg", "_DN.jpg" };

	//Load the 6 Skybox m_textures
	for (int i = 0; i < 6; i++)
	{
		std::string fName = fileName;
		fName += endings[i];

		IDirect3DTexture9* newTexture = NULL;
		D3DXCreateTextureFromFile(m_pDevice, fName.c_str(), &newTexture);
		m_textures.push_back(newTexture);
	}

	//Create Mesh
	if (FAILED(D3DXCreateMeshFVF(12, 24, D3DXMESH_MANAGED, SKYBOX_VERTEX::FVF, m_pDevice, &m_pMesh)))
		debug.Print("Failed to create SKYBOX m_pMesh");

	//Create vertices
	SKYBOX_VERTEX* v = 0;
	m_pMesh->LockVertexBuffer(0, (void**)&v);

	{
		D3DXVECTOR3 corners[8] = { D3DXVECTOR3(-size,  size,  size),
								D3DXVECTOR3(size,  size,  size),
								D3DXVECTOR3(-size,  size, -size),
								D3DXVECTOR3(size,  size, -size),
								D3DXVECTOR3(-size, -size,  size),
								D3DXVECTOR3(size, -size,  size),
								D3DXVECTOR3(-size, -size, -size),
								D3DXVECTOR3(size, -size, -size) };

		//Up Face
		v[0] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(0.0f, 0.0f));
		v[1] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(1.0f, 0.0f));
		v[2] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(0.0f, 1.0f));
		v[3] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(1.0f, 1.0f));

		//Front Face
		v[4] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(0.0f, 0.0f));
		v[5] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(1.0f, 0.0f));
		v[6] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(0.0f, 1.0f));
		v[7] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(1.0f, 1.0f));

		//Back Face
		v[8] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(0.0f, 0.0f));
		v[9] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(1.0f, 0.0f));
		v[10] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(0.0f, 1.0f));
		v[11] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(1.0f, 1.0f));

		//Right Face
		v[12] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(0.0f, 0.0f));
		v[13] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(1.0f, 0.0f));
		v[14] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(0.0f, 1.0f));
		v[15] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(1.0f, 1.0f));

		//Left Face
		v[16] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(0.0f, 0.0f));
		v[17] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(1.0f, 0.0f));
		v[18] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(0.0f, 1.0f));
		v[19] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(1.0f, 1.0f));

		//Down Face
		v[20] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(0.0f, 0.0f));
		v[21] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(1.0f, 0.0f));
		v[22] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(0.0f, 1.0f));
		v[23] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(1.0f, 1.0f));
	}

	m_pMesh->UnlockVertexBuffer();

	//Calculate Indices
	WORD* ind = 0;
	m_pMesh->LockIndexBuffer(0, (void**)&ind);

	int index = 0;
	for (int quad = 0; quad < 6; quad++)
	{
		//First face
		ind[index++] = quad * 4;
		ind[index++] = quad * 4 + 1;
		ind[index++] = quad * 4 + 2;

		//Second Face
		ind[index++] = quad * 4 + 1;
		ind[index++] = quad * 4 + 3;
		ind[index++] = quad * 4 + 2;
	}

	m_pMesh->UnlockIndexBuffer();

	//Set Attributes
	DWORD* att = 0;
	m_pMesh->LockAttributeBuffer(0, &att);

	//Set each quad to its own sub mesh
	for (int i = 0; i < 12; i++)
		att[i] = i / 2;

	m_pMesh->UnlockAttributeBuffer();

	//Set material
	memset(&m_white, 0, sizeof(D3DMATERIAL9));
	m_white.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

SKYBOX::~SKYBOX()
{
	//Release m_textures
	for (int i = 0; i < m_textures.size(); i++)
		if (m_textures[i] != NULL)
			m_textures[i]->Release();
	m_textures.clear();

	//Release mesh
	if (m_pMesh)m_pMesh->Release();
}

void SKYBOX::Render(D3DXVECTOR3 cameraPos)
{
	//Set Renderstates
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, false);

	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	//Set material
	m_pDevice->SetMaterial(&m_white);

	//Move skybox to center on cameraPos
	D3DXMATRIX position;
	D3DXMatrixTranslation(&position, cameraPos.x, cameraPos.y, cameraPos.z);
	m_pDevice->SetTransform(D3DTS_WORLD, &position);

	//Render the six sides of the skybox
	for (int i = 0; i < 6; i++)
	{
		m_pDevice->SetTexture(0, m_textures[i]);
		m_pMesh->DrawSubset(i);
	}

	// Restore render states.
	m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, true);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}