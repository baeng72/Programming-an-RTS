#include "mouse.h"

//////////////////////////////////////////////////////////////////////////
//						MOUSE											//
//////////////////////////////////////////////////////////////////////////

MOUSE::MOUSE()
{
	m_iType = 0;
	m_pMouseTexture = NULL;
	m_pMouseDevice = NULL;
	m_fSpeed = 1.5f;
	m_dwBlock = GetTickCount();
}

MOUSE::~MOUSE()
{
	if (m_pMouseDevice != NULL)
		m_pMouseDevice->Release();

	if (m_pMouseTexture != NULL)
		m_pMouseTexture->Release();
}

void MOUSE::InitMouse(IDirect3DDevice9* Dev, HWND wnd)
{
	try
	{
		m_pDevice = Dev;

		//Load texture and init sprite
		D3DXCreateTextureFromFile(m_pDevice, "cursor/cursor.dds", &m_pMouseTexture);
		D3DXCreateSprite(m_pDevice, &m_pSprite);

		//Get directinput object
		LPDIRECTINPUT8 directInput = NULL;

		DirectInput8Create(GetModuleHandle(NULL),	// Retrieves the application handle
			0x0800,					// v.8.0	
			IID_IDirectInput8,		// Interface ID
			(VOID**)&directInput,	// Our DirectInput Object
			NULL);

		//Acquire Default System mouse
		directInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, NULL);
		m_pMouseDevice->SetDataFormat(&c_dfDIMouse);
		m_pMouseDevice->SetCooperativeLevel(wnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		m_pMouseDevice->Acquire();

		//Get viewport size and init mouse location
		D3DVIEWPORT9 v;
		m_pDevice->GetViewport(&v);
		m_viewport.left = v.X;
		m_viewport.top = v.Y;
		m_viewport.right = v.X + v.Width;
		m_viewport.bottom = v.Y + v.Height;

		x = v.X + v.Width / 2.0f;
		y = v.Y + v.Height / 2.0f;

		//Release Direct Input Object
		directInput->Release();
	}
	catch (...)
	{
		debug.Print("Error in MOUSE::InitMouse()");
	}
}

bool MOUSE::ClickLeft()
{
	if (GetTickCount() < m_dwBlock)return false;
	return m_mouseState.rgbButtons[0];
}

bool MOUSE::ClickRight()
{
	if (GetTickCount() < m_dwBlock)return false;
	return m_mouseState.rgbButtons[1];
}

bool MOUSE::WheelUp()
{
	if (GetTickCount() < m_dwBlock)return false;
	return m_mouseState.lZ > 0.0f;
}

bool MOUSE::WheelDown()
{
	if (GetTickCount() < m_dwBlock)return false;
	return m_mouseState.lZ < 0.0f;
}

bool MOUSE::Over(RECT dest)
{
	if (x < dest.left || x > dest.right)return false;
	if (y < dest.top || y > dest.bottom)return false;
	return true;
}

bool MOUSE::PressInRect(RECT dest)
{
	return ClickLeft() && Over(dest);
}

void MOUSE::Update()
{
	//Retrieve mouse state
	ZeroMemory(&m_mouseState, sizeof(DIMOUSESTATE));
	m_pMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &m_mouseState);

	//Update pointer
	x += m_mouseState.lX * m_fSpeed;
	y += m_mouseState.lY * m_fSpeed;

	//Keep mouse pointer within window
	if (x < m_viewport.left)	x = m_viewport.left;
	if (y < m_viewport.top)	y = m_viewport.top;
	if (x > m_viewport.right)	x = m_viewport.right;
	if (y > m_viewport.bottom)	y = m_viewport.bottom;
}

void MOUSE::Paint()
{
	if (m_pMouseTexture != NULL)
	{
		RECT src[] = { {0,0,20,20}, {0,20,20,40}, {20,20,40,40}, {0,40,20,60}, {20,40,40,60} };

		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		D3DXVECTOR3 vec = D3DXVECTOR3(x, y, 0.0f);
		m_pSprite->Draw(m_pMouseTexture, &src[m_iType], NULL, &vec, 0xffffffff);
		m_pSprite->End();
	}
}

void MOUSE::DisableInput(int millisec)
{
	m_dwBlock = GetTickCount() + millisec;
}