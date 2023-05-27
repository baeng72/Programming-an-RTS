#ifndef _CAMERA
#define _CAMERA

#include <d3dx9.h>
//#include "mouse.h"
#include "mesh.h"
#include "debug.h"

class CAMERA {
	friend class APPLICATION;
	friend class OBJECT;
public:
	//Init Camera
	CAMERA();
	void Init(IDirect3DDevice9* Dev);

	//Movement
	void Scroll(D3DXVECTOR3 vec);	//Move Focus
	void Pitch(float f);			//Change B-angle
	void Yaw(float f);				//Change A-angle
	void Zoom(float f);				//Change FOV
	void ChangeRadius(float f);		//Change Radius... douh

	void Update(float timeDelta);

	//Calculate Matrices
	D3DXMATRIX GetViewMatrix();
	D3DXMATRIX GetProjectionMatrix();

private:

	IDirect3DDevice9* m_pDevice;
	float m_alpha, m_beta, m_radius, m_fov;
	D3DXVECTOR3 m_eye, m_focus;
};

#endif
