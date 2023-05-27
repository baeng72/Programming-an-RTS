#include "terrain.h"
#include "camera.h"

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

HRESULT PATCH::CreateMesh(TERRAIN& ter, RECT source, IDirect3DDevice9* Dev)
{
	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	try
	{
		m_pDevice = Dev;
		m_mapRect = source;

		int width = source.right - source.left;
		int height = source.bottom - source.top;
		int nrVert = (width + 1) * (height + 1);
		int nrTri = width * height * 2;

		if (FAILED(D3DXCreateMeshFVF(nrTri, nrVert, D3DXMESH_MANAGED, TERRAINVertex::FVF, m_pDevice, &m_pMesh)))
		{
			debug.Print("Couldn't create mesh for PATCH");
			return E_FAIL;
		}

		m_BBox.max = D3DXVECTOR3(-10000.0f, -10000.0f, -10000.0f);
		m_BBox.min = D3DXVECTOR3(10000.0f, 10000.0f, 10000.0f);

		//Create vertices
		TERRAINVertex* ver = 0;
		m_pMesh->LockVertexBuffer(0, (void**)&ver);
		for (int z = source.top, z0 = 0; z <= source.bottom; z++, z0++)
			for (int x = source.left, x0 = 0; x <= source.right; x++, x0++)
			{
				MAPTILE* tile = ter.GetTile(x, z);

				D3DXVECTOR3 pos = D3DXVECTOR3(x, tile->m_height, -z);
				D3DXVECTOR2 alphaUV = D3DXVECTOR2(x / (float)ter.m_size.x, z / (float)ter.m_size.y);		//Alpha UV
				D3DXVECTOR2 colorUV = alphaUV * 8.0f;													//Color UV

				ver[z0 * (width + 1) + x0] = TERRAINVertex(pos, ter.GetNormal(x, z), alphaUV, colorUV);

				//Calculate bounding box bounds...
				if (pos.x < m_BBox.min.x)m_BBox.min.x = pos.x;
				if (pos.x > m_BBox.max.x)m_BBox.max.x = pos.x;
				if (pos.y < m_BBox.min.y)m_BBox.min.y = pos.y;
				if (pos.y > m_BBox.max.y)m_BBox.max.y = pos.y;
				if (pos.z < m_BBox.min.z)m_BBox.min.z = pos.z;
				if (pos.z > m_BBox.max.z)m_BBox.max.z = pos.z;
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
	m_pMapTiles = NULL;
}

void TERRAIN::Init(IDirect3DDevice9* Dev, INTPOINT _size)
{
	m_pDevice = Dev;
	m_size = _size;
	m_pHeightMap = NULL;

	if (m_pMapTiles != NULL)	//Clear old maptiles
		delete[] m_pMapTiles;

	//Create maptiles
	m_pMapTiles = new MAPTILE[m_size.x * m_size.y];
	memset(m_pMapTiles, 0, sizeof(MAPTILE) * m_size.x * m_size.y);

	//Clear old textures
	for (int i = 0; i < m_diffuseMaps.size(); i++)
		m_diffuseMaps[i]->Release();
	m_diffuseMaps.clear();

	//Load textures
	IDirect3DTexture9* grass = NULL, * mount = NULL, * snow = NULL;
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/grass.jpg", &grass)))debug.Print("Could not load grass.jpg");
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/mountain.jpg", &mount)))debug.Print("Could not load mountain.jpg");
	if (FAILED(D3DXCreateTextureFromFile(Dev, "textures/snow.jpg", &snow)))debug.Print("Could not load snow.jpg");
	m_diffuseMaps.push_back(grass);
	m_diffuseMaps.push_back(mount);
	m_diffuseMaps.push_back(snow);
	m_pAlphaMap = NULL;
	m_pLightMap = NULL;

	// Init font
	D3DXCreateFont(m_pDevice, 40, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial Black", &m_pProgressFont);

	//Load pixel & vertex shaders
	m_dirToSun = D3DXVECTOR3(1.0f, 0.6f, 0.5f);
	D3DXVec3Normalize(&m_dirToSun, &m_dirToSun);

	m_terrainPS.Init(Dev, "shaders/terrain.ps", PIXEL_SHADER);
	m_terrainVS.Init(Dev, "shaders/terrain.vs", VERTEX_SHADER);
	m_vsMatW = m_terrainVS.GetConstant("matW");
	m_vsMatVP = m_terrainVS.GetConstant("matVP");
	m_vsDirToSun = m_terrainVS.GetConstant("DirToSun");

	m_objectsPS.Init(Dev, "shaders/objects.ps", PIXEL_SHADER);
	m_objectsVS.Init(Dev, "shaders/objects.vs", VERTEX_SHADER);
	m_objMatW = m_objectsVS.GetConstant("matW");
	m_objMatVP = m_objectsVS.GetConstant("matVP");
	m_objDirToSun = m_objectsVS.GetConstant("DirToSun");
	m_objMapSize = m_objectsVS.GetConstant("mapSize");

	//Create white material	
	m_mtrl.Ambient = m_mtrl.Specular = m_mtrl.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtrl.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

	GenerateRandomTerrain(9);
}

void TERRAIN::Release()
{
	try
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
	catch (...)
	{
		debug.Print("Error in TERRAIN::Release()");
	}
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	try
	{
		Release();

		//Create two heightmaps and multiply them
		m_pHeightMap = new HEIGHTMAP(m_size, 20.0f);
		HEIGHTMAP hm2(m_size, 2.0f);
		HEIGHTMAP hm3(m_size, 2.0f);

		m_pHeightMap->CreateRandomHeightMap(rand() % 2000, 1.0f, 0.7f, 7);
		hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.8f, 3);

		//Load 4 player filter
		hm3.LoadFromFile(m_pDevice, "heightmaps/four_players.jpg");
		hm2.Cap(hm2.m_maxHeight * 0.4f);

		*m_pHeightMap *= hm2;
		*m_pHeightMap *= hm3;
		hm2.Release();
		hm3.Release();

		//Add objects
		HEIGHTMAP hm4(m_size, 1.0f);
		hm4.CreateRandomHeightMap(rand() % 1000, 5.5f, 0.9f, 7);

		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				if (m_pHeightMap->GetHeight(x, y) == 0.0f && hm4.GetHeight(x, y) > 0.7f && rand() % 12 == 0)
					AddObject(0, INTPOINT(x, y));	//Tree
				else if (m_pHeightMap->GetHeight(x, y) >= 1.0f && hm4.GetHeight(x, y) > 0.9f && rand() % 30 == 0)
					AddObject(1, INTPOINT(x, y));	//Stone
			}

		hm4.Release();

		InitPathfinding();
		CreatePatches(numPatches);

		CalculateAlphaMaps();
		CalculateLightMap(false);
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

		//Create new patches
		for (int y = 0; y < numPatches; y++)
		{
			Progress("Creating Terrain Mesh", y / (float)numPatches);

			for (int x = 0; x < numPatches; x++)
			{
				RECT r = { x * (m_size.x - 1) / (float)numPatches,
						y * (m_size.y - 1) / (float)numPatches,
						(x + 1) * (m_size.x - 1) / (float)numPatches,
						(y + 1) * (m_size.y - 1) / (float)numPatches };

				PATCH* p = new PATCH();
				p->CreateMesh(*this, r, m_pDevice);
				m_patches.push_back(p);
			}
		}
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::CreatePatches()");
	}
}

void TERRAIN::CalculateAlphaMaps()
{
	Progress("Creating Alpha Map", 0.0f);

	//Clear old alpha maps
	if (m_pAlphaMap != NULL)
		m_pAlphaMap->Release();

	//Create new alpha map
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
				int terrain_x = m_size.x * (x / (float)(sRect.Pitch / 4.0f));
				int terrain_y = m_size.y * (y / (float)(sRect.Pitch / 4.0f));
				MAPTILE* tile = GetTile(terrain_x, terrain_y);

				if (tile != NULL && tile->m_type == i)
					bytes[y * sRect.Pitch + x * 4 + i] = 255;
			}

	//Unlock the texture
	m_pAlphaMap->UnlockRect(0);

	//D3DXSaveTextureToFile("alpha.bmp", D3DXIFF_BMP, m_pAlphaMap, NULL);
}

void TERRAIN::CalculateLightMap(bool allWhite)
{
	try
	{
		//Clear old alpha maps
		if (m_pLightMap != NULL)
			m_pLightMap->Release();

		//Create new light map
		D3DXCreateTexture(m_pDevice, 256, 256, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, &m_pLightMap);

		//Lock the texture
		D3DLOCKED_RECT sRect;
		m_pLightMap->LockRect(0, &sRect, NULL, NULL);
		BYTE* bytes = (BYTE*)sRect.pBits;
		memset(bytes, 255, 256 * sRect.Pitch);		//Clear texture to white

		if (allWhite)
		{
			m_pLightMap->UnlockRect(0);
			return;
		}

		for (int y = 0; y < sRect.Pitch; y++)
		{
			Progress("Calculating Lightmap", y / (float)sRect.Pitch);

			for (int x = 0; x < sRect.Pitch; x++)
			{
				float terrain_x = (float)m_size.x * (x / (float)(sRect.Pitch));
				float terrain_z = (float)m_size.y * (y / (float)(sRect.Pitch));

				//Find patch that the terrain_x, terrain_z is over
				bool done = false;
				for (int p = 0; p < m_patches.size() && !done; p++)
				{
					RECT mr = m_patches[p]->m_mapRect;

					//Focus within patch maprect or not?
					if (terrain_x >= mr.left && terrain_x < mr.right &&
						terrain_z >= mr.top && terrain_z < mr.bottom)
					{
						// Collect only the closest intersection
						RAY rayTop(D3DXVECTOR3(terrain_x, 10000.0f, -terrain_z), D3DXVECTOR3(0.0f, -1.0f, 0.0f));
						float dist = rayTop.Intersect(m_patches[p]->m_pMesh);

						if (dist >= 0.0f)
						{
							RAY ray(D3DXVECTOR3(terrain_x, 10000.0f - dist + 0.01f, -terrain_z), m_dirToSun);

							for (int p2 = 0; p2 < m_patches.size() && !done; p2++)
								if (ray.Intersect(m_patches[p2]->m_BBox) >= 0)
								{
									if (ray.Intersect(m_patches[p2]->m_pMesh) >= 0)	//In shadow
									{
										done = true;
										bytes[y * sRect.Pitch + x] = 128;
									}
								}

							done = true;
						}
					}
				}
			}
		}

		//Smooth lightmap		
		for (int i = 0; i < 3; i++)
		{
			Progress("Smoothing the Lightmap", i / 3.0f);

			BYTE* tmpBytes = new BYTE[sRect.Pitch * sRect.Pitch];
			memcpy(tmpBytes, sRect.pBits, sRect.Pitch * sRect.Pitch);

			for (int y = 1; y < sRect.Pitch - 1; y++)
				for (int x = 1; x < sRect.Pitch - 1; x++)
				{
					long index = y * sRect.Pitch + x;
					BYTE b1 = bytes[index];
					BYTE b2 = bytes[index - 1];
					BYTE b3 = bytes[index - sRect.Pitch];
					BYTE b4 = bytes[index + 1];
					BYTE b5 = bytes[index + sRect.Pitch];

					tmpBytes[index] = (BYTE)((b1 + b2 + b3 + b4 + b5) / 5);
				}

			memcpy(sRect.pBits, tmpBytes, sRect.Pitch * sRect.Pitch);
			delete[] tmpBytes;
		}

		//Unlock the texture
		m_pLightMap->UnlockRect(0);

		//D3DXSaveTextureToFile("light.bmp", D3DXIFF_BMP, m_pLightMap, NULL);
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::CalculateLightMap()");
	}
}

D3DXVECTOR3 TERRAIN::GetNormal(int x, int y)
{
	//Neighboring map nodes (D, B, C, F, H, G)
	INTPOINT mp[] = { INTPOINT(x - 1, y),   INTPOINT(x, y - 1),
					 INTPOINT(x + 1, y - 1), INTPOINT(x + 1, y),
					 INTPOINT(x, y + 1),   INTPOINT(x - 1, y + 1) };

	//if there's an invalid map node return (0, 1, 0)
	if (!Within(mp[0]) || !Within(mp[1]) || !Within(mp[2]) ||
		!Within(mp[3]) || !Within(mp[4]) || !Within(mp[5]))
		return D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	//Calculate the normals of the 6 neighboring planes
	D3DXVECTOR3 normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 6; i++)
	{
		D3DXPLANE plane;
		D3DXVECTOR3 vec1 = GetWorldPos(INTPOINT(x, y));
		D3DXVECTOR3 vec2 = GetWorldPos(mp[i]);
		D3DXVECTOR3 vec3 = GetWorldPos(mp[(i + 1) % 6]);
		D3DXPlaneFromPoints(&plane,
			&vec1,
			&vec2,
			&vec3);

		normal += D3DXVECTOR3(plane.a, plane.b, plane.c);
	}

	D3DXVec3Normalize(&normal, &normal);
	return normal;
}

void TERRAIN::AddObject(int type, INTPOINT mappos)
{
	D3DXVECTOR3 pos = D3DXVECTOR3(mappos.x, m_pHeightMap->GetHeight(mappos), -mappos.y);
	D3DXVECTOR3 rot = D3DXVECTOR3((rand() % 1000 / 1000.0f) * 0.13f, (rand() % 1000 / 1000.0f) * 3.0f, (rand() % 1000 / 1000.0f) * 0.13);

	float sca_xz = (rand() % 1000 / 1000.0f) * 0.5f + 0.5f;
	float sca_y = (rand() % 1000 / 1000.0f) * 1.0f + 0.5f;
	D3DXVECTOR3 sca = D3DXVECTOR3(sca_xz, sca_y, sca_xz);

	m_objects.push_back(OBJECT(type, mappos, pos, rot, sca));
}

void TERRAIN::Render(CAMERA& camera)
{
	//Set render states		
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	m_pDevice->SetTexture(0, m_pAlphaMap);
	m_pDevice->SetTexture(1, m_diffuseMaps[0]);		//Grass
	m_pDevice->SetTexture(2, m_diffuseMaps[1]);		//Mountain
	m_pDevice->SetTexture(3, m_diffuseMaps[2]);		//Snow
	m_pDevice->SetTexture(4, m_pLightMap);			//Lightmap
	m_pDevice->SetMaterial(&m_mtrl);

	D3DXMATRIX world, vp = camera.GetViewMatrix() * camera.GetProjectionMatrix();
	D3DXMatrixIdentity(&world);
	m_pDevice->SetTransform(D3DTS_WORLD, &world);

	//Set vertex shader variables
	m_terrainVS.SetMatrix(m_vsMatW, world);
	m_terrainVS.SetMatrix(m_vsMatVP, vp);
	m_terrainVS.SetVector3(m_vsDirToSun, m_dirToSun);

	m_terrainVS.Begin();
	m_terrainPS.Begin();

	for (int p = 0; p < m_patches.size(); p++)
		if (!camera.Cull(m_patches[p]->m_BBox))
			m_patches[p]->Render();

	m_terrainPS.End();
	m_terrainVS.End();

	m_pDevice->SetTexture(1, NULL);
	m_pDevice->SetTexture(2, NULL);
	m_pDevice->SetTexture(3, NULL);
	m_pDevice->SetTexture(4, NULL);

	//Render Objects
	m_objectsVS.SetMatrix(m_objMatW, world);
	m_objectsVS.SetMatrix(m_objMatVP, vp);
	m_objectsVS.SetVector3(m_objDirToSun, m_dirToSun);
	m_objectsVS.SetVector3(m_objMapSize, D3DXVECTOR3(m_size.x, m_size.y, 0.0f));

	m_pDevice->SetTexture(1, m_pLightMap);		//Lightmap

	m_objectsVS.Begin();
	m_objectsPS.Begin();

	for (int i = 0; i < m_objects.size(); i++)
		if (!camera.Cull(m_objects[i].m_BBox))
		{
			D3DXMATRIX m = m_objects[i].m_meshInstance.GetWorldMatrix();
			m_objectsVS.SetMatrix(m_objMatW, m);
			m_objects[i].Render();
		}

	m_objectsVS.End();
	m_objectsPS.End();
}

void TERRAIN::Progress(std::string text, float prc)
{
	m_pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);
	m_pDevice->BeginScene();

	RECT rc = { 200, 250, 600, 300 };
	m_pProgressFont->DrawText(NULL, text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_NOCLIP, 0xff000000);

	//Progress bar
	D3DRECT r;
	r.x1 = 200; r.x2 = 600;
	r.y1 = 300; r.y2 = 340;
	m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L);
	r.x1 = 202; r.x2 = 598;
	r.y1 = 302; r.y2 = 338;
	m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);
	r.x1 = 202; r.x2 = 202 + 396 * prc;
	r.y1 = 302; r.y2 = 338;
	m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff00ff00, 1.0f, 0L);

	m_pDevice->EndScene();
	m_pDevice->Present(0, 0, 0, 0);
}

bool TERRAIN::Within(INTPOINT p)
{
	return p.x >= 0 && p.y >= 0 && p.x < m_size.x&& p.y < m_size.y;
}

void TERRAIN::InitPathfinding()
{
	try
	{
		//Read maptile heights & types from heightmap
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				MAPTILE* tile = GetTile(x, y);
				if (m_pHeightMap != NULL)tile->m_height = m_pHeightMap->GetHeight(x, y);
				tile->m_mappos = INTPOINT(x, y);

				if (tile->m_height < 0.3f)		 tile->m_type = 0;	//Grass
				else if (tile->m_height < 7.0f) tile->m_type = 1;	//Stone
				else						 tile->m_type = 2;	//Snow
			}

		//Calculate tile cost as a function of the height variance
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				MAPTILE* tile = GetTile(x, y);

				if (tile != NULL)
				{
					//Possible neighbors
					INTPOINT p[] = { INTPOINT(x - 1, y - 1), INTPOINT(x, y - 1), INTPOINT(x + 1, y - 1),
									INTPOINT(x - 1, y),					  INTPOINT(x + 1, y),
									INTPOINT(x - 1, y + 1), INTPOINT(x, y + 1), INTPOINT(x + 1, y + 1) };

					float variance = 0.0f;
					int nr = 0;

					//For each neighbor
					for (int i = 0; i < 8; i++)
						if (Within(p[i]))
						{
							MAPTILE* neighbor = GetTile(p[i]);

							if (neighbor != NULL)
							{
								float v = neighbor->m_height - tile->m_height;
								variance += (v * v);
								nr++;
							}
						}

					//Cost = height variance
					variance /= (float)nr;
					tile->m_cost = variance + 0.1f;
					if (tile->m_cost > 1.0f)tile->m_cost = 1.0f;

					//If the tile cost is less than 1.0f, then we can walk on the tile
					tile->m_walkable = tile->m_cost < 0.5f;
				}
			}

		//Make maptiles with objects on them not walkable
		for (int i = 0; i < m_objects.size(); i++)
		{
			MAPTILE* tile = GetTile(m_objects[i].m_mappos);
			if (tile != NULL)
			{
				tile->m_walkable = false;
				tile->m_cost = 1.0f;
			}
		}

		//Connect m_pMapTiles using the neightbors[] pointers
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				MAPTILE* tile = GetTile(x, y);
				if (tile != NULL && tile->m_walkable)
				{
					//Clear old connections
					for (int i = 0; i < 8; i++)
						tile->m_pNeightbors[i] = NULL;

					//Possible neighbors
					INTPOINT p[] = { INTPOINT(x - 1, y - 1), INTPOINT(x, y - 1), INTPOINT(x + 1, y - 1),
									INTPOINT(x - 1, y),					  INTPOINT(x + 1, y),
									INTPOINT(x - 1, y + 1), INTPOINT(x, y + 1), INTPOINT(x + 1, y + 1) };

					//For each neighbor
					for (int i = 0; i < 8; i++)
						if (Within(p[i]))
						{
							MAPTILE* neighbor = GetTile(p[i]);

							//Connect tiles if the neighbor is walkable
							if (neighbor != NULL && neighbor->m_walkable)
								tile->m_pNeightbors[i] = neighbor;
						}
				}
			}

		CreateTileSets();
	}
	catch (...)
	{
		debug.Print("Error in InitPathfinding()");
	}
}

void TERRAIN::UpdatePathfinding(RECT* r)
{
	if (r == NULL)
	{
		InitPathfinding();
		return;
	}

	//Connect maptiles using the neightbors[] pointers
	for (int y = r->top; y <= r->bottom; y++)
		for (int x = r->left; x <= r->right; x++)
		{
			MAPTILE* tile = GetTile(x, y);
			if (tile != NULL && tile->m_walkable)
			{
				//Clear old connections
				for (int i = 0; i < 8; i++)
					tile->m_pNeightbors[i] = NULL;

				//Possible neighbors
				INTPOINT p[] = { INTPOINT(x - 1, y - 1), INTPOINT(x, y - 1), INTPOINT(x + 1, y - 1),
								INTPOINT(x - 1, y),					  INTPOINT(x + 1, y),
								INTPOINT(x - 1, y + 1), INTPOINT(x, y + 1), INTPOINT(x + 1, y + 1) };

				//For each neighbor
				for (int i = 0; i < 8; i++)
					if (Within(p[i]))
					{
						MAPTILE* neighbor = GetTile(p[i]);

						//Connect tiles if the neighbor is walkable
						if (neighbor != NULL && neighbor->m_walkable)
							tile->m_pNeightbors[i] = neighbor;
					}
			}
		}

	CreateTileSets();
}

void TERRAIN::CreateTileSets()
{
	try
	{
		int setNo = 0;
		for (int y = 0; y < m_size.y; y++)		//Set a unique set for each tile...
			for (int x = 0; x < m_size.x; x++)
				m_pMapTiles[x + y * m_size.x].m_set = setNo++;

		bool changed = true;
		while (changed)
		{
			changed = false;

			for (int y = 0; y < m_size.y; y++)
				for (int x = 0; x < m_size.x; x++)
				{
					MAPTILE* tile = GetTile(x, y);

					//Find the lowest m_set of a neighbor
					if (tile != NULL && tile->m_walkable)
					{
						for (int i = 0; i < 8; i++)
							if (tile->m_pNeightbors[i] != NULL &&
								tile->m_pNeightbors[i]->m_walkable &&
								tile->m_pNeightbors[i]->m_set < tile->m_set)
							{
								changed = true;
								tile->m_set = tile->m_pNeightbors[i]->m_set;
							}
					}
				}
		}
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::CreateTileSets()");
	}
}

float H(INTPOINT a, INTPOINT b)
{
	//return abs(a.x - b.x) + abs(a.y - b.y);
	return a.Distance(b);
}

std::vector<INTPOINT> TERRAIN::GetPath(INTPOINT start, INTPOINT goal, bool considerUnits)
{
	try
	{
		//Check that the two points are within the bounds of the map
		MAPTILE* startTile = GetTile(start);
		MAPTILE* goalTile = GetTile(goal);

		if (!Within(start) || !Within(goal) || start == goal || startTile == NULL || goalTile == NULL)
			return std::vector<INTPOINT>();

		//Check if a path exists
		if (!startTile->m_walkable || !goalTile->m_walkable || startTile->m_set != goalTile->m_set)
			return std::vector<INTPOINT>();

		//Init Search
		long numTiles = m_size.x * m_size.y;
		for (long l = 0; l < numTiles; l++)
		{
			m_pMapTiles[l].f = m_pMapTiles[l].g = INT_MAX;		//Clear F,G
			m_pMapTiles[l].open = m_pMapTiles[l].closed = false;	//Reset Open and Closed
		}

		std::vector<MAPTILE*> open;				//Create Our Open list
		startTile->g = 0;						//Init our starting point (SP)
		startTile->f = H(start, goal);
		startTile->open = true;
		open.push_back(startTile);				//Add SP to the Open list

		bool found = false;					// Search as long as a path hasnt been found,
		while (!found && !open.empty())		// or there is no more tiles to search
		{
			MAPTILE* best = open[0];        // Find the best tile (i.e. the lowest F value)
			int bestPlace = 0;
			for (int i = 1; i < open.size(); i++)
				if (open[i]->f < best->f)
				{
					best = open[i];
					bestPlace = i;
				}

			if (best == NULL)break;			//No path found

			open[bestPlace]->open = false;
			//open.erase(&open[bestPlace]);	// Take the best node out of the Open list
			std::vector<MAPTILE*>::iterator it = open.begin() + bestPlace;
			open.erase(it);

			if (best->m_mappos == goal)		//If the goal has been found
			{
				std::vector<INTPOINT> p, p2;
				MAPTILE* point = best;

				while (point->m_mappos != start)	// Generate path
				{
					p.push_back(point->m_mappos);
					point = point->m_pParent;

					if (p.size() > 500)		//Too long path, something is wrong
						return std::vector<INTPOINT>();
				}

				for (int i = p.size() - 1; i != 0; i--)	// Reverse path
					p2.push_back(p[i]);
				p2.push_back(goal);
				return p2;
			}
			else
			{
				for (int i = 0; i < 8; i++)					// otherwise, check the neighbors of the
					if (best->m_pNeightbors[i] != NULL)	// best tile
						if (!considerUnits || best->m_pNeightbors[i]->m_pMapObject == NULL)
						{
							bool inList = false;		// Generate new G and F value
							float newG = best->g + 1.0f;
							float d = H(best->m_mappos, best->m_pNeightbors[i]->m_mappos);
							float newF = newG + H(best->m_pNeightbors[i]->m_mappos, goal) + best->m_pNeightbors[i]->m_cost * 5.0f * d;

							if (best->m_pNeightbors[i]->open || best->m_pNeightbors[i]->closed)
							{
								if (newF < best->m_pNeightbors[i]->f)	// If the new F value is lower
								{
									best->m_pNeightbors[i]->g = newG;	// update the values of this tile
									best->m_pNeightbors[i]->f = newF;
									best->m_pNeightbors[i]->m_pParent = best;
								}
								inList = true;
							}

							if (!inList)			// If the neighbor tile isn't in the Open or Closed list
							{
								best->m_pNeightbors[i]->f = newF;		//Set the values
								best->m_pNeightbors[i]->g = newG;
								best->m_pNeightbors[i]->m_pParent = best;
								best->m_pNeightbors[i]->open = true;
								open.push_back(best->m_pNeightbors[i]);	//Add it to the open list	
							}
						}

				best->closed = true;		//The best tile has now been searched, add it to the Closed list
			}
		}

		return std::vector<INTPOINT>();		//No path found, return an empty path

	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::GetPath()");
		return std::vector<INTPOINT>();
	}
}

MAPTILE* TERRAIN::GetTile(int x, int y)
{
	if (m_pMapTiles == NULL)return NULL;

	try
	{
		if (Within(INTPOINT(x, y)))
			return &m_pMapTiles[x + y * m_size.x];
		else return NULL;
	}
	catch (...)
	{
		return NULL;
	}
}

void TERRAIN::SaveTerrain(char fileName[])
{
	try
	{
		std::ofstream out(fileName, std::ios::binary);		//Binary format

		if (out.good())
		{
			out.write((char*)&m_size, sizeof(INTPOINT));	//Write map size

			//Write all the maptile information needed to recreate the map
			for (int y = 0; y < m_size.y; y++)
				for (int x = 0; x < m_size.x; x++)
				{
					MAPTILE* tile = GetTile(x, y);
					out.write((char*)&tile->m_type, sizeof(int));			//type
					out.write((char*)&tile->m_height, sizeof(float));		//Height
				}

			//Write all the objects
			int numObjects = m_objects.size();
			out.write((char*)&numObjects, sizeof(int));	 //Num Objects
			for (int i = 0; i < m_objects.size(); i++)
			{
				out.write((char*)&m_objects[i].m_type, sizeof(int));				//m_type
				out.write((char*)&m_objects[i].m_mappos, sizeof(INTPOINT));			//mappos
				out.write((char*)&m_objects[i].m_meshInstance.m_pos, sizeof(D3DXVECTOR3));	//Pos
				out.write((char*)&m_objects[i].m_meshInstance.m_rot, sizeof(D3DXVECTOR3));	//Rot
				out.write((char*)&m_objects[i].m_meshInstance.m_sca, sizeof(D3DXVECTOR3));	//Sca
			}
		}

		out.close();
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::SaveTerrain()");
	}
}

void TERRAIN::LoadTerrain(char fileName[])
{
	try
	{
		std::ifstream in(fileName, std::ios::binary);		//Binary format

		if (in.good())
		{
			Release();	//Release all terrain resources

			in.read((char*)&m_size, sizeof(INTPOINT));	//read map size

			if (m_pMapTiles != NULL)	//Clear old maptiles
				delete[] m_pMapTiles;

			//Create new maptiles
			m_pMapTiles = new MAPTILE[m_size.x * m_size.y];
			memset(m_pMapTiles, 0, sizeof(MAPTILE) * m_size.x * m_size.y);


			//Read the maptile information
			for (int y = 0; y < m_size.y; y++)
				for (int x = 0; x < m_size.x; x++)
				{
					MAPTILE* tile = GetTile(x, y);
					in.read((char*)&tile->m_type, sizeof(int));			//type
					in.read((char*)&tile->m_height, sizeof(float));		//Height
				}

			//Read number of objects
			int numObjects = 0;
			in.read((char*)&numObjects, sizeof(int));
			for (int i = 0; i < numObjects; i++)
			{
				int type = 0;
				INTPOINT mp;
				D3DXVECTOR3 p, r, s;

				in.read((char*)&type, sizeof(int));			//type
				in.read((char*)&mp, sizeof(INTPOINT));		//mappos
				in.read((char*)&p, sizeof(D3DXVECTOR3));	//Pos
				in.read((char*)&r, sizeof(D3DXVECTOR3));	//Rot
				in.read((char*)&s, sizeof(D3DXVECTOR3));	//Sca

				m_objects.push_back(OBJECT(type, mp, p, r, s));
			}

			//Recreate Terrain
			InitPathfinding();
			CreatePatches(3);
			CalculateAlphaMaps();
			CalculateLightMap(true);
		}

		in.close();
	}
	catch (...)
	{
		debug.Print("Error in TERRAIN::LoadTerrain()");
	}
}

D3DXVECTOR3 TERRAIN::GetWorldPos(INTPOINT mappos)
{
	if (!Within(mappos))return D3DXVECTOR3(0, 0, 0);
	MAPTILE* tile = GetTile(mappos);
	return D3DXVECTOR3(mappos.x, tile->m_height, -mappos.y);
}

INTPOINT TERRAIN::GetClosestFreeTile(INTPOINT to, INTPOINT from)
{
	for (int i = 0; i < 5; i++)	//Search radius
	{
		std::vector<MAPTILE*> tiles;

		//Get tiles in current search radius
		for (int y = to.y - i; y <= to.y + i; y++)
		{
			tiles.push_back(GetTile(to.x - i, y));
			tiles.push_back(GetTile(to.x + i, y));
		}

		for (int x = to.x - i + 1; x <= to.x + i - 1; x++)
		{
			tiles.push_back(GetTile(x, to.y - i));
			tiles.push_back(GetTile(x, to.y + i));
		}

		//Find closest tile...
		MAPTILE* closest = NULL;
		float dist = 10000.0f;

		for (int t = 0; t < tiles.size(); t++)
			if (tiles[t] != NULL)
				if (tiles[t]->m_walkable && tiles[t]->m_pMapObject == NULL)
				{
					float d = from.Distance(tiles[t]->m_mappos);
					if (d < dist)
					{
						dist = d;
						closest = tiles[t];
					}
				}

		if (closest != NULL)
			return closest->m_mappos;
	}

	return to;
}