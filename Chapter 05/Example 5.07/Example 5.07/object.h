#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"
#include "camera.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define MECH 0

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot);
	void Render(CAMERA* camera, long& noFaces, int& noObjects);

	MESHINSTANCE m_meshInstances[3];
	int m_type;
	BBOX m_BBox;
};


#endif