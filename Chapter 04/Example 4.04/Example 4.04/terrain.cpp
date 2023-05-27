#include "terrain.h"

const DWORD TERRAINVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;

//////////////////////////////////////////////////////////////////////////////////////////
//									PATCH												//
//////////////////////////////////////////////////////////////////////////////////////////

PATCH::PATCH()
{
	m_pDevice = NULL;
	m_pMesh = NULL;
}
PATCH::~PATCH()
{
	Release();
}

void PATCH::Release()
{
	if (m_pMesh != NULL)
		m_pMesh->Release();
	m_pMesh = NULL;
}

HRESULT PATCH::CreateMesh(HEIGHTMAP& hm, RECT source, IDirect3DDevice9* Dev, int index)
{
	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	try
	{
		m_pDevice = Dev;

		int width = source.right - source.left;
		int height = source.bottom - source.top;
		int nrVert = (width + 1) * (height + 1);
		int nrTri = width * height * 2;

		if (FAILED(D3DXCreateMeshFVF(nrTri, nrVert, D3DXMESH_MANAGED, TERRAINVertex::FVF, m_pDevice, &m_pMesh)))
		{
			debug.Print("Couldn't create mesh for PATCH");
			return E_FAIL;
		}

		//Create vertices
		TERRAINVertex* ver = 0;
		m_pMesh->LockVertexBuffer(0, (void**)&ver);
		for (int z = source.top, z0 = 0; z <= source.bottom; z++, z0++)
			for (int x = source.left, x0 = 0; x <= source.right; x++, x0++)
			{
				//Calculate vertex color
				float prc = hm.m_pHeightMap[x + z * hm.m_size.x] / hm.m_maxHeight;
				int red = 255 * prc;
				int green = 255 * (1.0f - prc);

				D3DCOLOR col;

				if (index % 2 == 0)		//Invert color depending on what patch it is...
					col = D3DCOLOR_ARGB(255, red, green, 0);
				else col = D3DCOLOR_ARGB(255, green, red, 0);

				//Extract height (and position) from heightmap
				D3DXVECTOR3 pos = D3DXVECTOR3(x, hm.m_pHeightMap[x + z * hm.m_size.x], -z);

				//Set new vertex
				ver[z0 * (width + 1) + x0] = TERRAINVertex(pos, col);
			}
		m_pMesh->UnlockVertexBuffer();

		//Calculate Indices
		WORD* ind = 0;
		m_pMesh->LockIndexBuffer(0, (void**)&ind);
		int index = 0;

		for (int z = source.top, z0 = 0; z < source.bottom; z++, z0++)
			for (int x = source.left, x0 = 0; x < source.right; x++, x0++)
			{
				//Triangle 1
				ind[index++] = z0 * (width + 1) + x0;
				ind[index++] = z0 * (width + 1) + x0 + 1;
				ind[index++] = (z0 + 1) * (width + 1) + x0;

				//Triangle 2
				ind[index++] = (z0 + 1) * (width + 1) + x0;
				ind[index++] = z0 * (width + 1) + x0 + 1;
				ind[index++] = (z0 + 1) * (width + 1) + x0 + 1;
			}

		m_pMesh->UnlockIndexBuffer();

		//Set Attributes
		DWORD* att = 0;
		m_pMesh->LockAttributeBuffer(0, &att);
		memset(att, 0, sizeof(DWORD) * nrTri);
		m_pMesh->UnlockAttributeBuffer();

		//Compute normals
		D3DXComputeNormals(m_pMesh, NULL);

		ver = 0;
		m_pMesh->LockVertexBuffer(0, (void**)&ver);
		TERRAINVertex a = *ver;
		m_pMesh->UnlockVertexBuffer();
	}
	catch (...)
	{
		debug.Print("Error in PATCH::CreateMesh()");
		return E_FAIL;
	}

	return S_OK;
}

void PATCH::Render()
{
	//Draw mesh
	if (m_pMesh != NULL)
		m_pMesh->DrawSubset(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//									TERRAIN												//
//////////////////////////////////////////////////////////////////////////////////////////

TERRAIN::TERRAIN()
{
	m_pDevice = NULL;
}

void TERRAIN::Init(IDirect3DDevice9* Dev, INTPOINT _size)
{
	m_pDevice = Dev;
	m_size = _size;
	m_pHeightMap = NULL;
	GenerateRandomTerrain(3);
}

void TERRAIN::Release()
{
	for (int i = 0; i < m_patches.size(); i++)
		if (m_patches[i] != NULL)
			m_patches[i]->Release();

	m_patches.clear();

	if (m_pHeightMap != NULL)
	{
		m_pHeightMap->Release();
		delete m_pHeightMap;
		m_pHeightMap = NULL;
	}
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	try
	{
		Release();

		//Create two heightmaps and multiply them
		m_pHeightMap = new HEIGHTMAP(m_size, 15.0f);
		HEIGHTMAP hm2(m_size, 30.0f);

		m_pHeightMap->CreateRandomHeightMap(rand() % 2000, 2.5f, 0.5f, 8);
		hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.7f, 3);
		hm2.Cap(hm2.m_maxHeight * 0.4f);

		*m_pHeightMap *= hm2;
		hm2.Release();

		CreatePatches(numPatches);
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::GenerateRandomTerrain()");
	}
}

void TERRAIN::CreatePatches(int numPatches)
{
	try
	{
		//Clear any old m_patches
		for (int i = 0; i < m_patches.size(); i++)
			if (m_patches[i] != NULL)
				m_patches[i]->Release();
		m_patches.clear();

		if (m_pHeightMap == NULL)return;

		//Create new patches
		for (int y = 0; y < numPatches; y++)
			for (int x = 0; x < numPatches; x++)
			{
				RECT r = { x * (m_size.x - 1) / (float)numPatches,
						y * (m_size.y - 1) / (float)numPatches,
						(x + 1) * (m_size.x - 1) / (float)numPatches,
						(y + 1) * (m_size.y - 1) / (float)numPatches };

				PATCH* p = new PATCH();
				p->CreateMesh(*m_pHeightMap, r, m_pDevice, x + y);
				m_patches.push_back(p);
			}
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::CreatePatches()");
	}
}

void TERRAIN::Render()
{
	//Set render states
	m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	//Render Patches
	for (int i = 0; i < m_patches.size(); i++)
		m_patches[i]->Render();
}