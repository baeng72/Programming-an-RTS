#ifndef _CAMERA
#define _CAMERA

#include <d3dx9.h>
#include "mouse.h"
#include "debug.h"

class CAMERA {
	friend class APPLICATION;
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

	//Calculate Eye position etc
	void Update(MOUSE& mouse, float timeDelta);
	void CalculateFrustum(D3DXMATRIX view, D3DXMATRIX projection);
	bool Cull(BBOX bBox);
	bool Cull(BSPHERE bSphere);

	//Calculate Matrices
	D3DXMATRIX GetViewMatrix();
	D3DXMATRIX GetProjectionMatrix();

	//Public varialbles
	D3DXVECTOR3 m_eye;

private:

	IDirect3DDevice9* m_pDevice;
	float m_alpha, m_beta, m_radius, m_fov;
	D3DXVECTOR3 m_focus, m_right, m_look;

	//6-planes to store our view frustum
	D3DXPLANE m_frustum[6];
};

#endif
