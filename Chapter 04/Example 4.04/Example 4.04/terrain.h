#ifndef _TERRAIN_
#define _TERRAIN_

#include <d3dx9.h>
#include <vector>
#include "heightmap.h"
#include "debug.h"

struct TERRAINVertex
{
	TERRAINVertex() {}
	TERRAINVertex(D3DXVECTOR3 pos, D3DCOLOR col)
	{
		position = pos;
		color = col;
		normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	}

	D3DXVECTOR3 position, normal;
	D3DCOLOR color;

	static const DWORD FVF;
};

struct PATCH {
	PATCH();
	~PATCH();
	void Release();
	HRESULT CreateMesh(HEIGHTMAP& hm, RECT source, IDirect3DDevice9* Dev, int index);
	void Render();

	IDirect3DDevice9* m_pDevice;
	ID3DXMesh* m_pMesh;
};

class TERRAIN {
	friend class APPLICATION;
public:
	TERRAIN();
	void Init(IDirect3DDevice9* Dev, INTPOINT _size);
	void Release();
	void GenerateRandomTerrain(int numPatches);
	void CreatePatches(int numPatches);
	void Render();

private:

	INTPOINT m_size;
	IDirect3DDevice9* m_pDevice;

	HEIGHTMAP* m_pHeightMap;
	std::vector<PATCH*> m_patches;
};

#endif