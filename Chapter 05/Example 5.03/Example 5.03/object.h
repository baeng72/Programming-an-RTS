#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define DRAGON 0
#define BOB 1
#define RING 2

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca);
	void Render();
	void RenderBoundingVolume(int typ);

	MESHINSTANCE m_meshInstance;
	int m_type;
	std::string m_name;

	BBOX m_BBox;
	BSPHERE m_BSphere;
	ID3DXMesh* m_boxMesh, * m_sphereMesh;
};


#endif
