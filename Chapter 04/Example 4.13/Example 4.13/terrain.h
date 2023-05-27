#ifndef _TERRAIN_
#define _TERRAIN_

#include <d3dx9.h>
#include <vector>
#include "heightmap.h"
#include "debug.h"
#include "shader.h"
#include "object.h"

struct TERRAINVertex
{
	TERRAINVertex() {}
	TERRAINVertex(D3DXVECTOR3 pos, D3DXVECTOR2 _uv1, D3DXVECTOR2 _uv2)
	{
		position = pos;
		normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		uv1 = _uv1;
		uv2 = _uv2;
	}

	D3DXVECTOR3 position, normal;
	D3DXVECTOR2 uv1, uv2;

	static const DWORD FVF;
};

struct PATCH {
	PATCH();
	~PATCH();
	void Release();
	HRESULT CreateMesh(HEIGHTMAP& hm, RECT source, IDirect3DDevice9* Dev);
	void Render();

	IDirect3DDevice9* m_pDevice;
	ID3DXMesh* m_pMesh;
};

struct MAPTILE {
	MAPTILE()	//Set everything to 0
	{
		m_type = m_set = 0;
		m_height = m_cost = 0.0f;
		m_walkable = false;
		m_pParent = NULL;

		for (int i = 0; i < 8; i++)
			m_pNeighbors[i] = NULL;
	}

	int m_type, m_set;
	float m_height, m_cost;
	bool m_walkable;
	MAPTILE* m_pNeighbors[8];

	// Pathfinding variables
	INTPOINT m_mappos;
	float f, g;
	bool open, closed;
	MAPTILE* m_pParent;
};

class TERRAIN {
	friend class APPLICATION;
public:
	TERRAIN();
	void Init(IDirect3DDevice9* Dev, INTPOINT _size);
	void Release();
	void GenerateRandomTerrain(int numPatches);
	void CreatePatches(int numPatches);
	void CalculateAlphaMaps();
	void AddObject(int type, INTPOINT mappos);
	void Render();

	//Pathfinding
	bool Within(INTPOINT p);	//Test if a point is within the bounds of the terrain
	void InitPathfinding();
	void CreateTileSets();
	std::vector<INTPOINT> GetPath(INTPOINT start, INTPOINT goal);
	MAPTILE* GetTile(int x, int y);
	MAPTILE* GetTile(INTPOINT p) { return GetTile(p.x, p.y); }

	//Public variables
	MAPTILE* m_pMapTiles;

private:

	INTPOINT m_size;
	IDirect3DDevice9* m_pDevice;

	HEIGHTMAP* m_pHeightMap;
	std::vector<PATCH*> m_patches;
	std::vector<IDirect3DTexture9*> m_diffuseMaps;
	std::vector<OBJECT> m_objects;
	IDirect3DTexture9* m_pAlphaMap;
	SHADER m_terrainPS;

	D3DMATERIAL9 m_mtrl;
};

#endif