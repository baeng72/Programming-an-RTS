#ifndef _MESH
#define _MESH

#include <d3dx9.h>
#include <vector>

class MESH
{
	friend class MESHINSTANCE;
public:

	MESH();
	MESH(const char fName[], IDirect3DDevice9* Dev);
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

class MESHINSTANCE {
public:
	MESHINSTANCE();
	MESHINSTANCE(MESH* meshPtr);
	void Render();

	void SetMesh(MESH* m) { m_pMesh = m; }
	void SetPosition(D3DXVECTOR3 p) { m_pos = p; }
	void SetRotation(D3DXVECTOR3 r) { m_rot = r; }
	void SetScale(D3DXVECTOR3 s) { m_sca = s; }

	MESH* m_pMesh;
	D3DXVECTOR3 m_pos, m_rot, m_sca;
};

#endif