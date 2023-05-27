#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <vector>
#include "debug.h"
#include "mesh.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define OBJ_TREE 0
#define OBJ_STONE 1

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca);
	void Render();

private:
	MESHINSTANCE m_meshInstance;
	int m_type;
};


#endif