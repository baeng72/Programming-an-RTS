#include "terrain.h"

const DWORD TERRAINVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2;

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

HRESULT PATCH::CreateMesh(HEIGHTMAP& hm, RECT source, IDirect3DDevice9* Dev)
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
				D3DXVECTOR3 pos = D3DXVECTOR3(x, hm.m_pHeightMap[x + z * hm.m_size.x], -z);
				D3DXVECTOR2 alphaUV = D3DXVECTOR2(x / (float)hm.m_size.x, z / (float)hm.m_size.y);		//Alpha UV
				D3DXVECTOR2 colorUV = alphaUV * 8.0f;													//Color UV
				ver[z0 * (width + 1) + x0] = TERRAINVertex(pos, alphaUV, colorUV);
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
		DWORD* att = 0, a = 0;
		m_pMesh->LockAttributeBuffer(0, &att);
		memset(att, 0, sizeof(DWORD) * nrTri);
		m_pMesh->UnlockAttributeBuffer();

		//Compute normals
		D3DXComputeNormals(m_pMesh, NULL);
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

	//Load textures
	IDirect3DTexture9* grass = NULL, * mount = NULL, * snow = NULL;
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/grass.jpg", &grass)))debug.Print("Could not load grass.jpg");
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/mountain.jpg", &mount)))debug.Print("Could not load mountain.jpg");
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/snow.jpg", &snow)))debug.Print("Could not load snow.jpg");
	m_diffuseMaps.push_back(grass);
	m_diffuseMaps.push_back(mount);
	m_diffuseMaps.push_back(snow);
	m_pAlphaMap = NULL;

	//Load pixelshader
	m_terrainPS.Init(Dev, "Shaders/terrain.ps", PIXEL_SHADER);

	//Create white material	
	m_mtrl.Ambient = m_mtrl.Specular = m_mtrl.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtrl.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

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

	m_objects.clear();
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	try
	{
		Release();

		//Create two heightmaps and multiply them
		m_pHeightMap = new HEIGHTMAP(m_size, 20.0f);
		HEIGHTMAP hm2(m_size, 2.0f);

		m_pHeightMap->CreateRandomHeightMap(rand() % 2000, 2.0f, 0.5f, 8);
		hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.8f, 3);

		hm2.Cap(hm2.m_maxHeight * 0.4f);

		*m_pHeightMap *= hm2;
		hm2.Release();

		CreatePatches(numPatches);
		CalculateAlphaMaps();

		//Add m_objects
		HEIGHTMAP hm3(m_size, 1.0f);
		hm3.CreateRandomHeightMap(rand() % 1000, 5.5f, 0.9f, 7);

		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				if (m_pHeightMap->GetHeight(x, y) == 0.0f && hm3.GetHeight(x, y) > 0.7f && rand() % 6 == 0)
					AddObject(0, INTPOINT(x, y));	//Tree
				else if (m_pHeightMap->GetHeight(x, y) >= 1.0f && hm3.GetHeight(x, y) > 0.9f && rand() % 20 == 0)
					AddObject(1, INTPOINT(x, y));	//Stone
			}

		hm3.Release();
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

		//Create new m_patches
		for (int y = 0; y < numPatches; y++)
			for (int x = 0; x < numPatches; x++)
			{
				RECT r = { x * (m_size.x - 1) / (float)numPatches,
						  y * (m_size.y - 1) / (float)numPatches,
						(x + 1) * (m_size.x - 1) / (float)numPatches,
						(y + 1) * (m_size.y - 1) / (float)numPatches };

				PATCH* p = new PATCH();
				p->CreateMesh(*m_pHeightMap, r, m_pDevice);
				m_patches.push_back(p);
			}
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::CreatePatches()");
	}
}

void TERRAIN::CalculateAlphaMaps()
{
	//Clear old alpha maps
	if (m_pAlphaMap != NULL)
		m_pAlphaMap->Release();

	//height ranges...
	float min_range[] = { 0.0f, 1.0f, 15.0f };
	float max_range[] = { 2.0f, 16.0f, 21.0f };

	D3DXCreateTexture(m_pDevice, 128, 128, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pAlphaMap);

	//Lock the texture
	D3DLOCKED_RECT sRect;
	m_pAlphaMap->LockRect(0, &sRect, NULL, NULL);
	BYTE* bytes = (BYTE*)sRect.pBits;
	memset(bytes, 0, 128 * sRect.Pitch);		//Clear texture to black

	for (int i = 0; i < m_diffuseMaps.size(); i++)
		for (int y = 0; y < sRect.Pitch / 4; y++)
			for (int x = 0; x < sRect.Pitch / 4; x++)
			{
				int hm_x = m_pHeightMap->m_size.x * (x / (float)(sRect.Pitch / 4.0f));
				int hm_y = m_pHeightMap->m_size.y * (y / (float)(sRect.Pitch / 4.0f));
				float height = m_pHeightMap->m_pHeightMap[hm_x + hm_y * m_pHeightMap->m_size.x];

				BYTE* b = bytes + y * sRect.Pitch + x * 4 + i;
				if (height >= min_range[i] && height <= max_range[i])
					*b = 255;
				else *b = 0;
			}

	//Unlock the texture
	m_pAlphaMap->UnlockRect(0);

	//D3DXSaveTextureToFile("alpha.bmp", D3DXIFF_BMP, m_pAlphaMap, NULL);
}

void TERRAIN::AddObject(int type, INTPOINT mappos)
{
	D3DXVECTOR3 pos = D3DXVECTOR3(mappos.x, m_pHeightMap->GetHeight(mappos), -mappos.y);
	D3DXVECTOR3 rot = D3DXVECTOR3((rand() % 1000 / 1000.0f) * 0.13f, (rand() % 1000 / 1000.0f) * 3.0f, (rand() % 1000 / 1000.0f) * 0.13);

	float sca_xz = (rand() % 1000 / 1000.0f) * 0.5f + 0.5f;
	float sca_y = (rand() % 1000 / 1000.0f) * 1.0f + 0.5f;
	D3DXVECTOR3 sca = D3DXVECTOR3(sca_xz, sca_y, sca_xz);

	m_objects.push_back(OBJECT(type, pos, rot, sca));
}

void TERRAIN::Render()
{
	//Set render states		
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	m_pDevice->SetTexture(0, m_pAlphaMap);
	m_pDevice->SetTexture(1, m_diffuseMaps[0]);		//Grass
	m_pDevice->SetTexture(2, m_diffuseMaps[1]);		//Mountain
	m_pDevice->SetTexture(3, m_diffuseMaps[2]);		//Snow
	m_pDevice->SetMaterial(&m_mtrl);

	m_terrainPS.Begin();

	for (int p = 0; p < m_patches.size(); p++)
		m_patches[p]->Render();

	m_terrainPS.End();

	//Render Objects
	for (int i = 0; i < m_objects.size(); i++)
		m_objects[i].Render();
}