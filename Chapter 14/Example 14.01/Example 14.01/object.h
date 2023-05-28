#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <vector>
#include "debug.h"
#include "mesh.h"
#include "intpoint.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define OBJ_TREE 0
#define OBJ_STONE 1

class OBJECT {
	friend class TERRAIN;
public:
	OBJECT();
	OBJECT(int t, INTPOINT mp, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca);
	void Render();

private:
	INTPOINT m_mappos;
	MESHINSTANCE m_meshInstance;
	int m_type;
	BBOX m_BBox;
	bool m_visible;
};


#endif