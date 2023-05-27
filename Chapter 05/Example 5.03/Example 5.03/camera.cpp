#include "camera.h"

CAMERA::CAMERA()
{
	Init(NULL);
}

void CAMERA::Init(IDirect3DDevice9* Dev)
{
	m_pDevice = Dev;
	m_alpha = m_beta = 0.5f;
	m_radius = 10.0f;
	m_fov = D3DX_PI / 4.0f;

	m_eye = D3DXVECTOR3(50.0f, 50.0f, 50.0f);
	m_focus = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void CAMERA::Scroll(D3DXVECTOR3 vec)
{
	D3DXVECTOR3 newFocus = m_focus + vec;

	if (newFocus.x > -50.0f && newFocus.x < 50.0f &&
		newFocus.z > -50.0f && newFocus.z < 50.0f)
		m_focus = newFocus;
}

void CAMERA::Pitch(float f)
{
	m_beta += f;

	if (m_beta > D3DX_PI / 2.0f)m_beta = D3DX_PI / 2.0f - 0.01f;
	if (m_beta < 0.3f)m_beta = 0.3f;
}

void CAMERA::Yaw(float f)
{
	m_alpha += f;
	if (m_alpha > D3DX_PI * 2.0f)m_alpha -= D3DX_PI * 2.0f;
	if (m_alpha < -D3DX_PI * 2.0f)m_alpha += D3DX_PI * 2.0f;
}

void CAMERA::Zoom(float f)
{
	m_fov += f;

	if (m_fov < 0.1f)m_fov = 0.1f;
	if (m_fov > D3DX_PI / 2.0f)m_fov = D3DX_PI / 2.0f;
}

void CAMERA::ChangeRadius(float f)
{
	m_radius += f;

	if (m_radius < 5.0f)m_radius = 5.0f;
	if (m_radius > 100.0f)m_radius = 100.0f;
}

void CAMERA::Update(MOUSE& mouse, float timeDelta)
{
	//Restrict focus movment to the xz-plane
	m_right.y = m_look.y = 0.0f;
	D3DXVec3Normalize(&m_look, &m_look);
	D3DXVec3Normalize(&m_right, &m_right);

	//Move Focus (i.e. Scroll)
	if (mouse.x < mouse.m_viewport.left + 10)Scroll(-m_right * timeDelta * 5.0f);
	if (mouse.x > mouse.m_viewport.right - 10)Scroll(m_right * timeDelta * 5.0f);
	if (mouse.y < mouse.m_viewport.top + 10)Scroll(m_look * timeDelta * 5.0f);
	if (mouse.y > mouse.m_viewport.bottom - 10)Scroll(-m_look * timeDelta * 5.0f);

	//Move Camera (i.e. Change Angle)
	if (KEYDOWN(VK_LEFT))Yaw(-timeDelta);
	if (KEYDOWN(VK_RIGHT))Yaw(timeDelta);
	if (KEYDOWN(VK_UP))Pitch(timeDelta);
	if (KEYDOWN(VK_DOWN))Pitch(-timeDelta);

	//Zoom (i.e. change m_fov)
	if (KEYDOWN(VK_ADD))Zoom(-timeDelta);
	if (KEYDOWN(VK_SUBTRACT))Zoom(timeDelta);

	//Change m_radius
	if (mouse.WheelUp())  ChangeRadius(-1.0f);
	if (mouse.WheelDown())ChangeRadius(1.0f);

	//Calculate Eye Position
	float sideRadius = m_radius * cos(m_beta);
	float height = m_radius * sin(m_beta);

	m_eye = D3DXVECTOR3(m_focus.x + sideRadius * cos(m_alpha),
		m_focus.y + height,
		m_focus.z + sideRadius * sin(m_alpha));

	if (m_pDevice != NULL)
	{
		D3DXMATRIX view = GetViewMatrix();
		m_pDevice->SetTransform(D3DTS_VIEW, &view);
		D3DXMATRIX proj = GetProjectionMatrix();
		m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);
	}
}

D3DXMATRIX CAMERA::GetViewMatrix()
{
	D3DXMATRIX  matView;
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &m_eye, &m_focus, &up);

	m_right.x = matView(0, 0);
	m_right.y = matView(1, 0);
	m_right.z = matView(2, 0);
	D3DXVec3Normalize(&m_right, &m_right);

	m_look.x = matView(0, 2);
	m_look.y = matView(1, 2);
	m_look.z = matView(2, 2);
	D3DXVec3Normalize(&m_right, &m_right);

	return  matView;
}

D3DXMATRIX CAMERA::GetProjectionMatrix()
{
	D3DXMATRIX  matProj;
	float aspect = 800.0f / 600.0f;
	D3DXMatrixPerspectiveFovLH(&matProj, m_fov, aspect, 1.0f, 1000.0f);
	return matProj;
}