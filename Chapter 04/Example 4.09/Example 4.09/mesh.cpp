#include "mesh.h"
#include <fstream>
MESH::MESH()
{
	m_pDevice = NULL;
	m_pMesh = NULL;
}

MESH::MESH(char fName[], IDirect3DDevice9* Dev)
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

	//Set white material
	m_white.Ambient = m_white.Specular = m_white.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_white.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_white.Power = 1.0f;

	Release();

	//Load new mesh
	ID3DXBuffer* adjacencyBfr = NULL;
	ID3DXBuffer* materialBfr = NULL;
	DWORD noMaterials = NULL;

	if (FAILED(D3DXLoadMeshFromX(fName, D3DXMESH_MANAGED, m_pDevice,
		&adjacencyBfr, &materialBfr, NULL,
		&noMaterials, &m_pMesh)))
		return E_FAIL;

	D3DXMATERIAL* mtrls = (D3DXMATERIAL*)materialBfr->GetBufferPointer();

	//Extract and save materials
	for (int i = 0; i < noMaterials; i++)
	{
		m_materials.push_back(mtrls[i].MatD3D);

		//Load texture (if any)
		if (mtrls[i].pTextureFilename != NULL)
		{
			char textureFileName[90];
			strcpy(textureFileName, "meshes/");			
			strcat(textureFileName, mtrls[i].pTextureFilename);			
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

	return S_OK;
}

void MESH::Render()
{
	//For each Material (& texture) render a subset of the mesh
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