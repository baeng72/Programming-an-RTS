#include "mesh.h"

MESH::MESH()
{
	m_pDevice = NULL;
	m_pMesh = NULL;
}

MESH::MESH(const char fName[], IDirect3DDevice9* Dev)
{
	m_pDevice = Dev;
	m_pMesh = NULL;
	Load(fName, m_pDevice);
}

MESH::~MESH()
{
	Release();
}

HRESULT MESH::Load(const char fName[], IDirect3DDevice9* Dev)
{
	m_pDevice = Dev;

	//Set m_white material
	m_white.Ambient = m_white.Specular = m_white.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_white.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_white.Power = 1.0f;

	Release();

	//Load new mesh
	ID3DXBuffer* adjacencyBfr = NULL;
	ID3DXBuffer* materialBfr = NULL;
	DWORD noMaterials = NULL;

	if (FAILED(D3DXLoadMeshFromX(fName, D3DXMESH_MANAGED, m_pDevice, &adjacencyBfr, &materialBfr, NULL, &noMaterials, &m_pMesh)))
		return E_FAIL;

	D3DXMATERIAL* mtrls = (D3DXMATERIAL*)materialBfr->GetBufferPointer();

	for (int i = 0; i < noMaterials; i++)
	{
		m_materials.push_back(mtrls[i].MatD3D);

		if (mtrls[i].pTextureFilename != NULL)
		{
			char textureFileName[90];
			//strcpy(textureFileName, "meshes/");
			strcpy_s(textureFileName, sizeof(textureFileName), "meshes/");
			//strcat(textureFileName, mtrls[i].pTextureFilename);
			strcat_s(textureFileName, sizeof(textureFileName), mtrls[i].pTextureFilename);
			IDirect3DTexture9* newTexture = NULL;
			D3DXCreateTextureFromFile(m_pDevice, textureFileName, &newTexture);
			m_textures.push_back(newTexture);
		}
		else m_textures.push_back(NULL);
	}

	m_pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjacencyBfr->GetBufferPointer(), NULL, NULL, NULL);

	adjacencyBfr->Release();
	materialBfr->Release();
}

void MESH::Render()
{
	for (int i = 0; i < m_materials.size(); i++)
	{
		if (m_textures[i] != NULL)m_pDevice->SetMaterial(&m_white);
		else m_pDevice->SetMaterial(&m_materials[i]);
		m_pDevice->SetTexture(0, m_textures[i]);
		m_pMesh->DrawSubset(i);
	}
}

void MESH::Release()
{
	//Clear old mesh...
	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	//Clear textures and materials
	for (int i = 0; i < m_textures.size(); i++)
		if (m_textures[i] != NULL)
			m_textures[i]->Release();

	m_textures.clear();
	m_materials.clear();
}

MESHINSTANCE::MESHINSTANCE()
{
	m_pMesh = NULL;
	m_pos = m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

MESHINSTANCE::MESHINSTANCE(MESH* meshPtr)
{
	m_pMesh = meshPtr;
	m_pos = m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_sca = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

void MESHINSTANCE::Render()
{
	if (m_pMesh != NULL)
	{
		D3DXMATRIX p, r, s;
		D3DXMatrixTranslation(&p, m_pos.x, m_pos.y, m_pos.z);
		D3DXMatrixRotationYawPitchRoll(&r, m_rot.y, m_rot.x, m_rot.z);
		D3DXMatrixScaling(&s, m_sca.x, m_sca.y, m_sca.z);

		D3DXMATRIX world = s * r * p;
		m_pMesh->m_pDevice->SetTransform(D3DTS_WORLD, &world);

		m_pMesh->Render();
	}
}