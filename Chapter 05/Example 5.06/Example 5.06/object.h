#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <d3dx9.h>
#include <vector>
#include "functions.h"
#include "debug.h"
#include "mesh.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define TILE 0
#define HOUSE 1
#define HOUSE2 2
#define PARK 3

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca);
	void Render();

	MESHINSTANCE m_meshInstance;
	int m_type;
	BBOX m_bBox;
	BSPHERE m_bSphere;
	bool m_rendered;
};


#endif