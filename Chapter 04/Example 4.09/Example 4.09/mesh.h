#ifndef _MESH
#define _MESH

#include <d3dx9.h>
#include <vector>

class MESH
{
public:

	MESH();
	MESH(char fName[], IDirect3DDevice9* Dev);
	~MESH();
	HRESULT Load(const char fName[], IDirect3DDevice9* Dev);
	void Render();
	void Release();

private:

	IDirect3DDevice9* m_pDevice;
	ID3DXMesh* m_pMesh;
	std::vector<IDirect3DTexture9*> m_textures;
	std::vector<D3DMATERIAL9> m_materials;
	D3DMATERIAL9 m_white;
};

#endif