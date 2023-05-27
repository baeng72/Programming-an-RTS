#ifndef _CAMERA
#define _CAMERA

#include <d3dx9.h>
#include "debug.h"

class CAMERA {
public:
	//Init Camera
	CAMERA();
	void Init(IDirect3DDevice9* Dev);

	//Calculate Matrices
	D3DXMATRIX GetViewMatrix();
	D3DXMATRIX GetProjectionMatrix();

	IDirect3DDevice9* m_pDevice;
	float m_fov;
	D3DXVECTOR3 m_eye, m_focus;
};

#endif
