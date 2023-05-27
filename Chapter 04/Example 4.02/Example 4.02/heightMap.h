#include <d3dx9.h>
#include "intpoint.h"

struct PARTICLE {
	D3DXVECTOR3 position;
	D3DCOLOR color;
	static const DWORD FVF;
};

struct HEIGHTMAP
{
	//Functions
	HEIGHTMAP(IDirect3DDevice9* Dev, INTPOINT _size);
	~HEIGHTMAP();
	void Release();

	HRESULT LoadFromFile(char fileName[]);
	HRESULT CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves);

	HRESULT CreateParticles();
	void Render();
	D3DXVECTOR2 GetCentre() { return D3DXVECTOR2(m_size.x / 2.0f, m_size.y / 2.0f); }

	//variables
	INTPOINT m_size;
	float m_maxHeight;
	float* m_pHeightMap;

	//Vertex buffer for points
	IDirect3DVertexBuffer9* m_pVb;

	//Our device
	IDirect3DDevice9* m_pDevice;

	//Sprite
	ID3DXSprite* m_pSprite;
	IDirect3DTexture9* m_pHeightMapTexture;
};