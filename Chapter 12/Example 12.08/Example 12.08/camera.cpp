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
	m_focus = newFocus;
}

void CAMERA::Pitch(float f)
{
	m_beta += f;

	if (m_beta > (D3DX_PI / 2.0f) - 0.05f)m_beta = D3DX_PI / 2.0f - 0.05f;
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

	if (m_radius < 3.0f)m_radius = 3.0f;
	if (m_radius > 70.0f)m_radius = 70.0f;
}

void CAMERA::Update(float timeDelta)
{
	//Move Camera (i.e. Change Angle)
	if (KEYDOWN(VK_LEFT))Yaw(-timeDelta);
	if (KEYDOWN(VK_RIGHT))Yaw(timeDelta);
	if (KEYDOWN(VK_UP))Pitch(timeDelta);
	if (KEYDOWN(VK_DOWN))Pitch(-timeDelta);

	//Calculate Eye Position
	float sideRadius = m_radius * cos(m_beta);
	float height = m_radius * sin(m_beta);

	m_eye = D3DXVECTOR3(m_focus.x + sideRadius * cos(m_alpha),
		m_focus.y + height,
		m_focus.z + sideRadius * sin(m_alpha));

	if (m_pDevice != NULL)
	{
		D3DXMATRIX view = GetViewMatrix();
		D3DXMATRIX projection = GetProjectionMatrix();

		m_pDevice->SetTransform(D3DTS_VIEW, &view);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &projection);
	}
}

D3DXMATRIX CAMERA::GetViewMatrix()
{
	D3DXMATRIX  matView;
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &m_eye, &m_focus, &up);

	return  matView;
}

D3DXMATRIX CAMERA::GetProjectionMatrix()
{
	D3DXMATRIX  matProj;
	float aspect = 800.0f / 600.0f;
	D3DXMatrixPerspectiveFovLH(&matProj, m_fov, aspect, 1.0f, 1000.0f);
	return matProj;
}