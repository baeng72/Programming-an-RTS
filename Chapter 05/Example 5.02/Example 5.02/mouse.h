#ifndef _MOUSE
#define _MOUSE

#include <d3dx9.h>
#include <dinput.h>
#include "debug.h"
#include "intpoint.h"

class MOUSE : public INTPOINT {
public:
	MOUSE();
	~MOUSE();
	void InitMouse(IDirect3DDevice9* Device, HWND wnd);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(RECT dest);
	bool PressInRect(RECT dest);
	void Update();
	void Paint();

	float m_speed;
	int m_type;

private:
	LPDIRECTINPUTDEVICE8 m_pMouseDevice;
	DIMOUSESTATE m_mouseState;
	IDirect3DTexture9* m_pMouseTexture;
	LPD3DXSPRITE m_pSprite;
	RECT m_viewport;
};


#endif