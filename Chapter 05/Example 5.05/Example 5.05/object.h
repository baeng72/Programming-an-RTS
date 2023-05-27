#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"
#include "camera.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, float off);
	void Render();
	void Update(float deltaTime);
	void UpdateCameras();

	MESHINSTANCE m_meshInstance;
	int m_type;

	//Animation variables
	int m_activeWP, m_nextWP;
	float m_prc, m_speed, m_offset;

	//Camera variables
	std::vector<CAMERA> m_cameras;
	int m_activeCam;
	D3DXVECTOR3 m_direction;
};


#endif