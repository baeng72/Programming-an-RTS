#ifndef RTS_SKYBOX
#define RTS_SKYBOX

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"

class SKYBOX
{
public:
	SKYBOX(IDirect3DDevice9* Dev, const char fileName[], float size);
	~SKYBOX();
	void Render(D3DXVECTOR3 cameraPos);

private:
	IDirect3DDevice9* m_pDevice;
	std::vector<IDirect3DTexture9*> m_textures;
	ID3DXMesh* m_pMesh;
	D3DMATERIAL9 m_white;
};

#endif