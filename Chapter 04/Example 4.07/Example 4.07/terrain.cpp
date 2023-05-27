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
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	try
	{
		Release();

		//Create two heightmaps and multiply them
		m_pHeightMap = new HEIGHTMAP(m_size, 20.0f);
		HEIGHTMAP hm2(m_size, 20.0f);

		m_pHeightMap->CreateRandomHeightMap(rand() % 2000, 2.0f, 0.7f, 8);
		hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.8f, 3);

		hm2.Cap(hm2.m_fMaxHeight * 0.4f);

		*m_pHeightMap *= hm2;
		hm2.Release();

		CreatePatches(numPatches);
		CalculateAlphaMaps();
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
	for (int i = 0; i < m_alphaMaps.size(); i++)
		if (m_alphaMaps[i] != NULL)
			m_alphaMaps[i]->Release();
	m_alphaMaps.clear();

	//height ranges...
	float min_range[] = { 0.0f, 1.0f, 15.0f };

	//Create one alpha map per diffuse map
	for (int i = 0; i < m_diffuseMaps.size(); i++)
	{
		//Create new texture
		IDirect3DTexture9* newTexture = NULL;
		D3DXCreateTexture(m_pDevice, 128, 128, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &newTexture);

		//Lock the texture
		D3DLOCKED_RECT sRect;
		newTexture->LockRect(0, &sRect, NULL, NULL);
		BYTE* bytes = (BYTE*)sRect.pBits;

		//For each pixel in the alphaMap
		for (int y = 0; y < sRect.Pitch / 4; y++)
			for (int x = 0; x < sRect.Pitch / 4; x++)
			{
				int hm_x = m_pHeightMap->m_size.x * (x / (float)(sRect.Pitch / 4));
				int hm_y = m_pHeightMap->m_size.y * (y / (float)(sRect.Pitch / 4));
				float height = m_pHeightMap->m_pHeightMap[hm_x + hm_y * m_pHeightMap->m_size.x];

				//Paint white if the height is within the correct range
				BYTE* b = bytes + y * sRect.Pitch + x * 4;
				if (height >= min_range[i])memset(b, 255, 4);
				else memset(b, 0, 4);
			}

		//Unlock the texture
		newTexture->UnlockRect(0);

		//char *fname[] = {"a1.bmp","a2.bmp","a3.bmp"};
		//D3DXSaveTextureToFile(fname[i], D3DXIFF_BMP, newTexture, NULL);

		m_alphaMaps.push_back(newTexture);
	}
}

void TERRAIN::Render()
{
	//Set render states		
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	//Alpha from alphaTextures, stage 0
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//Color from textures, stage 1
	m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

	//Set blend states
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_pDevice->SetMaterial(&m_mtrl);

	//Draw mesh once for each terrain type
	for (int i = 0; i < m_diffuseMaps.size(); i++)
	{
		m_pDevice->SetTexture(0, m_alphaMaps[i]);
		m_pDevice->SetTexture(1, m_diffuseMaps[i]);

		for (int p = 0; p < m_patches.size(); p++)
			m_patches[p]->Render();
	}
}