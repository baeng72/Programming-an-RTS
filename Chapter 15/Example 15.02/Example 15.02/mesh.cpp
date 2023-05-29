#include "mesh.h"

const DWORD ObjectVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

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

	return S_OK;
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

void MESH::Render(float prc)
{
	//Calculate the end subset to render
	int end = m_materials.size() * prc;

	for (int i = 0; i < m_materials.size() && i < end; i++)
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

D3DXMATRIX MESHINSTANCE::GetWorldMatrix()
{
	D3DXMATRIX p, r, s;
	D3DXMatrixTranslation(&p, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationYawPitchRoll(&r, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixScaling(&s, m_sca.x, m_sca.y, m_sca.z);

	D3DXMATRIX world = s * r * p;
	return world;
}

void MESHINSTANCE::Render()
{
	if (m_pMesh != NULL)
	{
		D3DXMATRIX world = GetWorldMatrix();
		m_pMesh->m_pDevice->SetTransform(D3DTS_WORLD, &world);
		m_pMesh->Render();
	}
}

void MESHINSTANCE::Render(float prc)
{
	if (m_pMesh != NULL)
	{
		D3DXMATRIX world = GetWorldMatrix();
		m_pMesh->m_pDevice->SetTransform(D3DTS_WORLD, &world);
		m_pMesh->Render(prc);
	}
}

BBOX MESHINSTANCE::GetBoundingBox()
{
	if (m_pMesh == NULL || m_pMesh->m_pMesh == NULL)return BBOX();

	if (m_pMesh->m_pMesh->GetFVF() != ObjectVertex::FVF)		// XYZ and NORMAL and UV
		return BBOX();

	BBOX bBox(D3DXVECTOR3(-10000.0f, -10000.0f, -10000.0f),
		D3DXVECTOR3(10000.0f, 10000.0f, 10000.0f));
	D3DXMATRIX World = GetWorldMatrix();

	//Lock vertex buffer of the object
	ObjectVertex* vertexBuffer = NULL;
	m_pMesh->m_pMesh->LockVertexBuffer(0, (void**)&vertexBuffer);

	//For each vertex in the mesh
	for (int i = 0; i < m_pMesh->m_pMesh->GetNumVertices(); i++)
	{
		//Transform vertex to world space using the MESHINSTANCE
		//world matrix, i.e. the position, rotation and scale
		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &vertexBuffer[i]._pos, &World);

		// Check if the vertex is outside the bounds
		// if so, then update the bounding volume
		if (pos.x < bBox.min.x)bBox.min.x = pos.x;
		if (pos.x > bBox.max.x)bBox.max.x = pos.x;
		if (pos.y < bBox.min.y)bBox.min.y = pos.y;
		if (pos.y > bBox.max.y)bBox.max.y = pos.y;
		if (pos.z < bBox.min.z)bBox.min.z = pos.z;
		if (pos.z > bBox.max.z)bBox.max.z = pos.z;
	}

	m_pMesh->m_pMesh->UnlockVertexBuffer();

	return bBox;
}

BSPHERE MESHINSTANCE::GetBoundingSphere()
{
	if (m_pMesh == NULL || m_pMesh->m_pMesh == NULL)return BSPHERE();
	if (m_pMesh->m_pMesh->GetFVF() != ObjectVertex::FVF)		// XYZ and NORMAL and UV
		return BSPHERE();

	BBOX bBox = GetBoundingBox();
	BSPHERE bSphere;
	D3DXMATRIX World = GetWorldMatrix();
	bSphere.center = (bBox.max + bBox.min) / 2.0f;

	ObjectVertex* vertexBuffer = NULL;
	m_pMesh->m_pMesh->LockVertexBuffer(0, (void**)&vertexBuffer);

	//Get radius
	for (int i = 0; i < m_pMesh->m_pMesh->GetNumVertices(); i++)
	{
		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &vertexBuffer[i]._pos, &World);
		D3DXVECTOR3 vec = (pos - bSphere.center);
		float l = D3DXVec3Length(&vec);
		if (l > bSphere.radius)
			bSphere.radius = l;
	}

	m_pMesh->m_pMesh->UnlockVertexBuffer();

	return bSphere;
}