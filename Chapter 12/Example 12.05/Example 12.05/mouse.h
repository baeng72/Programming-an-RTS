#ifndef _MOUSE
#define _MOUSE

#include <d3dx9.h>
#include <dinput.h>
#include "debug.h"
#include "intpoint.h"
#include "mesh.h"
#include "terrain.h"

struct RAY {
	RAY();
	RAY(D3DXVECTOR3 o, D3DXVECTOR3 d);

	//Our different intersection tests
	float Intersect(MESHINSTANCE iMesh);
	float Intersect(BBOX bBox);
	float Intersect(BSPHERE bSphere);
	float Intersect(ID3DXMesh* mesh);

	D3DXVECTOR3 org, dir;
};


class MOUSE : public INTPOINT {
	friend class CAMERA;
public:
	MOUSE();
	~MOUSE();
	void InitMouse(IDirect3DDevice9* Dev, HWND wnd);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(RECT dest);
	bool PressInRect(RECT dest);
	void Update();
	void Paint();
	RAY GetRay();

private:
	IDirect3DDevice9* m_pDevice;
	LPDIRECTINPUTDEVICE8 m_pMouseDevice;
	DIMOUSESTATE m_mouseState;
	IDirect3DTexture9* m_pMouseTexture;
	LPD3DXSPRITE m_pSprite;
	RECT m_viewport;
};


#endif