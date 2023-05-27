#include "mouse.h"

//////////////////////////////////////////////////////////////////////////
//						RAY												//
//////////////////////////////////////////////////////////////////////////

RAY::RAY()
{
	org = dir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

RAY::RAY(D3DXVECTOR3 o, D3DXVECTOR3 d)
{
	org = o;
	dir = d;
}

float RAY::Intersect(MESHINSTANCE iMesh)
{
	if (iMesh.m_pMesh == NULL)return -1.0f;
	return Intersect(iMesh.m_pMesh->m_pMesh);
}

float RAY::Intersect(BBOX bBox)
{
	if (D3DXBoxBoundProbe(&bBox.min, &bBox.max, &org, &dir)) {
		D3DXVECTOR3 vec = (((bBox.min + bBox.max) / 2.0f) - org);
		return D3DXVec3Length(&vec);
	}
	else return -1.0f;
}

float RAY::Intersect(BSPHERE bSphere)
{
	if (D3DXSphereBoundProbe(&bSphere.center, bSphere.radius, &org, &dir)) {
		D3DXVECTOR3 vec = (bSphere.center - org);
		return D3DXVec3Length(&vec);
	}
	else return -1.0f;
}

float RAY::Intersect(ID3DXMesh* mesh)
{
	if (mesh == NULL)return -1.0f;

	// Collect only the closest intersection
	BOOL hit;
	DWORD dwFace;
	float hitU, hitV, dist;
	D3DXIntersect(mesh, &org, &dir, &hit, &dwFace, &hitU, &hitV, &dist, NULL, NULL);

	if (hit) return dist;
	else return -1.0f;
}

//////////////////////////////////////////////////////////////////////////
//						MOUSE											//
//////////////////////////////////////////////////////////////////////////

MOUSE::MOUSE()
{
	m_pMouseTexture = NULL;
	m_pMouseDevice = NULL;
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
	return m_mouseState.rgbButtons[0];
}

bool MOUSE::ClickRight()
{
	return m_mouseState.rgbButtons[1];
}

bool MOUSE::WheelUp()
{
	return m_mouseState.lZ > 0.0f;
}

bool MOUSE::WheelDown()
{
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
	x += m_mouseState.lX * 1.0f;
	y += m_mouseState.lY * 1.0f;

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
		RECT src = { 0,0,20,20 };

		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		D3DXVECTOR3 pos = D3DXVECTOR3(x, y, 0.0f);
		m_pSprite->Draw(m_pMouseTexture, &src, NULL, &pos, 0xffffffff);
		m_pSprite->End();
	}
}

RAY MOUSE::GetRay()
{
	try
	{
		D3DXMATRIX projectionMatrix, viewMatrix, worldViewInverse, worldMatrix;

		m_pDevice->GetTransform(D3DTS_PROJECTION, &projectionMatrix);
		m_pDevice->GetTransform(D3DTS_VIEW, &viewMatrix);
		m_pDevice->GetTransform(D3DTS_WORLD, &worldMatrix);

		float width = m_viewport.right - m_viewport.left;
		float height = m_viewport.bottom - m_viewport.top;

		float angle_x = (((2.0f * x) / width) - 1.0f) / projectionMatrix(0, 0);
		float angle_y = (((-2.0f * y) / height) + 1.0f) / projectionMatrix(1, 1);

		RAY ray;
		ray.org = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		ray.dir = D3DXVECTOR3(angle_x, angle_y, 1.0f);

		D3DXMATRIX m = worldMatrix * viewMatrix;
		D3DXMatrixInverse(&worldViewInverse, 0, &m);
		D3DXVec3TransformCoord(&ray.org, &ray.org, &worldViewInverse);
		D3DXVec3TransformNormal(&ray.dir, &ray.dir, &worldViewInverse);
		D3DXVec3Normalize(&ray.dir, &ray.dir);

		return ray;
	}
	catch (...)
	{
		debug.Print("Error in MOUSE::GetRay()");
	}
}