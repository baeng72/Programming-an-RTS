#ifndef _MESH
#define _MESH

#include <d3dx9.h>
#include <vector>

struct BBOX {
	BBOX()
	{
		max = D3DXVECTOR3(-10000.0f, -10000.0f, -10000.0f);
		min = D3DXVECTOR3(10000.0f, 10000.0f, 10000.0f);
	}

	BBOX(D3DXVECTOR3 _max, D3DXVECTOR3 _min)
	{
		max = _max;
		min = _min;
	}

	D3DXVECTOR3 max, min;
};

struct BSPHERE {
	BSPHERE()
	{
		center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		radius = 0.0f;
	}

	BSPHERE(D3DXVECTOR3 _center, float _radius)
	{
		center = _center;
		radius = _radius;
	}

	D3DXVECTOR3 center;
	float radius;
};

struct ObjectVertex
{
	ObjectVertex() {}
	ObjectVertex(D3DXVECTOR3 pos, D3DXVECTOR3 norm, float u, float v)
	{
		_pos = pos;
		_norm = norm;
		_u = u;
		_v = v;
	}

	D3DXVECTOR3 _pos, _norm;
	float _u, _v;

	static const DWORD FVF;
};

class MESH
{
	friend class OBJECT;
	friend class MESHINSTANCE;
	friend struct RAY;
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
	friend class OBJECT;
	friend struct RAY;
public:
	MESHINSTANCE();
	MESHINSTANCE(MESH* meshPtr);
	void Render();

	void SetMesh(MESH* m) { m_pMesh = m; }
	void SetPosition(D3DXVECTOR3 p) { m_pos = p; }
	void SetRotation(D3DXVECTOR3 r) { m_rot = r; }
	void SetScale(D3DXVECTOR3 s) { m_sca = s; }

	D3DXMATRIX GetWorldMatrix();
	BBOX GetBoundingBox();
	BSPHERE GetBoundingSphere();

	MESH* m_pMesh;
	D3DXVECTOR3 m_pos, m_rot, m_sca;
};

#endif