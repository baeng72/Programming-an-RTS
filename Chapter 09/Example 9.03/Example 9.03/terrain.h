#ifndef _TERRAIN_
#define _TERRAIN_

#include <d3dx9.h>
#include <vector>
#include <fstream>
#include "heightmap.h"
#include "debug.h"
#include "shader.h"
#include "object.h"

class CAMERA;
class MAPOBJECT;

struct TERRAINVertex
{
	TERRAINVertex() {}
	TERRAINVertex(D3DXVECTOR3 pos, D3DXVECTOR3 norm, D3DXVECTOR2 _uv1, D3DXVECTOR2 _uv2)
	{
		position = pos;
		normal = norm;
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
	HRESULT CreateMesh(TERRAIN& ter, RECT source, IDirect3DDevice9* Dev);
	void Render();

	IDirect3DDevice9* m_pDevice;
	ID3DXMesh* m_pMesh;
	RECT m_mapRect;
	BBOX m_BBox;
};

struct MAPTILE {
	MAPTILE()	//Set everything to 0
	{
		m_type = m_set = 0;
		m_height = m_cost = 0.0f;
		m_walkable = false;
		m_pMapObject = NULL;
		m_pParent = NULL;

		for (int i = 0; i < 8; i++)
			m_pNeightbors[i] = NULL;
	}

	int m_type, m_set;
	float m_height, m_cost;
	bool m_walkable;
	MAPTILE* m_pNeightbors[8];
	MAPOBJECT* m_pMapObject;

	// Pathfinding variables
	INTPOINT m_mappos;
	float f, g;
	bool open, closed;
	MAPTILE* m_pParent;
};

class TERRAIN {
	friend struct PATCH;
	friend class MOUSE;
	friend class CAMERA;
	friend class APPLICATION;
public:
	TERRAIN();
	void Init(IDirect3DDevice9* Dev, INTPOINT _size);
	void Release();
	void GenerateRandomTerrain(int numPatches);
	void CreatePatches(int numPatches);
	void CalculateAlphaMaps();
	void CalculateLightMap(bool allWhite);
	D3DXVECTOR3 GetNormal(int x, int y);
	void AddObject(int type, INTPOINT mappos);
	void Render(CAMERA& camera);
	void Progress(std::string text, float prc);

	//Pathfinding
	bool Within(INTPOINT p);	//Test if a point is within the bounds of the terrain
	void InitPathfinding();
	void UpdatePathfinding(RECT* r);
	void CreateTileSets();
	std::vector<INTPOINT> GetPath(INTPOINT start, INTPOINT goal, bool considerUnits);
	MAPTILE* GetTile(int x, int y);
	MAPTILE* GetTile(INTPOINT p) { return GetTile(p.x, p.y); }
	D3DXVECTOR3 GetWorldPos(INTPOINT mappos);
	INTPOINT GetClosestFreeTile(INTPOINT to, INTPOINT from);

	//Save and Load Map
	void SaveTerrain(char fileName[]);
	void LoadTerrain(char fileName[]);

	//Public variables
	MAPTILE* m_pMapTiles;

private:

	INTPOINT m_size;
	IDirect3DDevice9* m_pDevice;
	ID3DXFont* m_pProgressFont;

	HEIGHTMAP* m_pHeightMap;
	std::vector<PATCH*> m_patches;
	std::vector<IDirect3DTexture9*> m_diffuseMaps;
	std::vector<OBJECT> m_objects;
	IDirect3DTexture9* m_pAlphaMap;
	IDirect3DTexture9* m_pLightMap;

	SHADER m_terrainPS, m_terrainVS;
	SHADER m_objectsPS, m_objectsVS;

	D3DXVECTOR3 m_dirToSun;
	D3DXHANDLE m_vsMatW, m_vsMatVP, m_vsDirToSun;
	D3DXHANDLE m_objMatW, m_objMatVP, m_objDirToSun, m_objMapSize;

	D3DMATERIAL9 m_mtrl;
};

#endif