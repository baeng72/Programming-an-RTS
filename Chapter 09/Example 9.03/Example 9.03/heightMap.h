#ifndef _HEIGHTMAP_
#define _HEIGHTMAP_

#include <d3dx9.h>
#include "intpoint.h"

struct HEIGHTMAP
{
	//Functions
	HEIGHTMAP(INTPOINT _size, float _maxHeight);
	~HEIGHTMAP();
	void Release();
	void operator*=(const HEIGHTMAP& rhs);

	HRESULT LoadFromFile(IDirect3DDevice9* Device, const char fileName[]);
	HRESULT CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves);
	void RaiseTerrain(RECT r, float f);
	void SmoothTerrain();
	void Cap(float capHeight);
	float GetHeight(int x, int y);
	float GetHeight(INTPOINT p);

	//variables
	INTPOINT m_size;
	float m_maxHeight;
	float* m_pHeightMap;
};

#endif
