#include "camera.h"
#include <iostream>
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
	if (m_beta < 0.5f)m_beta = 0.5f;
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

	if (m_radius < 2.0f)m_radius = 2.0f;
	if (m_radius > 70.0f)m_radius = 70.0f;
}

void CAMERA::Update(MOUSE& mouse, TERRAIN& terrain, float timeDelta)
{
	//Restrict focus movment to the xz-plane
	m_right.y = m_look.y = 0.0f;

	D3DXVec3Normalize(&m_look, &m_look);
	D3DXVec3Normalize(&m_right, &m_right);

	//Move Focus (i.e. Scroll)
	if (mouse.x < mouse.m_viewport.left + 10)	Scroll(-m_right * timeDelta * (4.0f + m_radius * 0.2f));
	if (mouse.x > mouse.m_viewport.right - 10)	Scroll(m_right * timeDelta * (4.0f + m_radius * 0.2f));
	if (mouse.y < mouse.m_viewport.top + 10)		Scroll(m_look * timeDelta * (4.0f + m_radius * 0.2f));
	if (mouse.y > mouse.m_viewport.bottom - 10)	Scroll(-m_look * timeDelta * (4.0f + m_radius * 0.2f));

	//Move Camera (i.e. Change Angle)
	if (KEYDOWN(VK_LEFT))Yaw(-timeDelta);
	if (KEYDOWN(VK_RIGHT))Yaw(timeDelta);
	if (KEYDOWN(VK_UP))Pitch(timeDelta);
	if (KEYDOWN(VK_DOWN))Pitch(-timeDelta);

	//Zoom (i.e. change fov)
	if (KEYDOWN(VK_ADD))Zoom(-timeDelta);
	if (KEYDOWN(VK_SUBTRACT))Zoom(timeDelta);

	//Change radius
	if (mouse.WheelUp())  ChangeRadius(-1.0f);
	if (mouse.WheelDown())ChangeRadius(1.0f);

	//Calculate Eye Position
	float sideRadius = m_radius * cos(m_beta);
	float height = m_radius * sin(m_beta);

	m_eye = D3DXVECTOR3(m_focus.x + sideRadius * cos(m_alpha),
		m_focus.y + height,
		m_focus.z + sideRadius * sin(m_alpha));

	//Have the focus follow the terrain heights
	//Find patch that the focus is over
	for (int p = 0; p < terrain.m_patches.size(); p++)
	{
		RECT mr = terrain.m_patches[p]->m_mapRect;

		//Focus within patch maprect or not?
		if (m_focus.x >= mr.left && m_focus.x < mr.right &&
			-m_focus.z >= mr.top && -m_focus.z < mr.bottom)
		{
			// Collect only the closest intersection
			BOOL hit;
			DWORD dwFace;
			float hitU, hitV, dist;
			D3DXVECTOR3 rayPos = D3DXVECTOR3(m_focus.x, 10000.0f, m_focus.z);
			D3DXVECTOR3 rayDir = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
			D3DXIntersect(terrain.m_patches[p]->m_pMesh, &rayPos,
				&rayDir, &hit, &dwFace, &hitU, &hitV,
				&dist, NULL, NULL);

			if (hit)m_focus.y = 10000.0f - dist;
		}
	}

	if (m_pDevice != NULL)
	{
		D3DXMATRIX view = GetViewMatrix();
		D3DXMATRIX projection = GetProjectionMatrix();

		m_pDevice->SetTransform(D3DTS_VIEW, &view);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &projection);

		CalculateFrustum(view, projection);
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
	D3DXVec3Normalize(&m_look, &m_look);

	return  matView;
}

D3DXMATRIX CAMERA::GetProjectionMatrix()
{
	D3DXMATRIX  matProj;
	float aspect = 800.0f / 600.0f;
	D3DXMatrixPerspectiveFovLH(&matProj, m_fov, aspect, 1.0f, 1000.0f);
	return matProj;
}

void CAMERA::CalculateFrustum(D3DXMATRIX view, D3DXMATRIX projection)
{
	try
	{
		// Get combined matrix
		D3DXMATRIX matComb;
		D3DXMatrixMultiply(&matComb, &view, &projection);

		// Left clipping plane
		m_frustum[0].a = matComb._14 + matComb._11;
		m_frustum[0].b = matComb._24 + matComb._21;
		m_frustum[0].c = matComb._34 + matComb._31;
		m_frustum[0].d = matComb._44 + matComb._41;

		// Right clipping plane 
		m_frustum[1].a = matComb._14 - matComb._11;
		m_frustum[1].b = matComb._24 - matComb._21;
		m_frustum[1].c = matComb._34 - matComb._31;
		m_frustum[1].d = matComb._44 - matComb._41;

		// Top clipping plane 
		m_frustum[2].a = matComb._14 - matComb._12;
		m_frustum[2].b = matComb._24 - matComb._22;
		m_frustum[2].c = matComb._34 - matComb._32;
		m_frustum[2].d = matComb._44 - matComb._42;

		// Bottom clipping plane 
		m_frustum[3].a = matComb._14 + matComb._12;
		m_frustum[3].b = matComb._24 + matComb._22;
		m_frustum[3].c = matComb._34 + matComb._32;
		m_frustum[3].d = matComb._44 + matComb._42;

		// Near clipping plane 
		m_frustum[4].a = matComb._13;
		m_frustum[4].b = matComb._23;
		m_frustum[4].c = matComb._33;
		m_frustum[4].d = matComb._43;

		// Far clipping plane 
		m_frustum[5].a = matComb._14 - matComb._13;
		m_frustum[5].b = matComb._24 - matComb._23;
		m_frustum[5].c = matComb._34 - matComb._33;
		m_frustum[5].d = matComb._44 - matComb._43;

		//Normalize planes
		for (int i = 0; i < 6; i++)
			D3DXPlaneNormalize(&m_frustum[i], &m_frustum[i]);
	}
	catch (...)
	{
		debug.Print("Error in CAMERA::CalculateFrustum()");
	}
}

bool CAMERA::Cull(BBOX bBox)
{
	try
	{
		//For each plane in the view frustum
		for (int f = 0; f < 6; f++)
		{
			D3DXVECTOR3 c1, c2;

			//Find furthest point (n1) & nearest point (n2) to the plane
			if (m_frustum[f].a > 0.0f) { c1.x = bBox.max.x; c2.x = bBox.min.x; }
			else { c1.x = bBox.min.x; c2.x = bBox.max.x; }
			if (m_frustum[f].b > 0.0f) { c1.y = bBox.max.y; c2.y = bBox.min.y; }
			else { c1.y = bBox.min.y; c2.y = bBox.max.y; }
			if (m_frustum[f].c > 0.0f) { c1.z = bBox.max.z; c2.z = bBox.min.z; }
			else { c1.z = bBox.min.z; c2.z = bBox.max.z; }

			float distance1 = m_frustum[f].a * c1.x + m_frustum[f].b * c1.y +
				m_frustum[f].c * c1.z + m_frustum[f].d;
			float distance2 = m_frustum[f].a * c2.x + m_frustum[f].b * c2.y +
				m_frustum[f].c * c2.z + m_frustum[f].d;

			//If both points are on the negative side of the plane, Cull!
			if (distance1 < 0.0f && distance2 < 0.0f)
				return true;
		}

		//Object is inside the volume
		return false;
	}
	catch (...)
	{
		debug.Print("Error in CAMERA::Cull()");
		return true;
	}
}

bool CAMERA::Cull(BSPHERE bSphere)
{
	try
	{
		//For each plane in the view frustum
		for (int f = 0; f < 6; f++)
		{
			D3DXVECTOR3 vec = D3DXVECTOR3(m_frustum[f].a, m_frustum[f].b, m_frustum[f].c);
			float distance = D3DXVec3Dot(&bSphere.center, &vec) + m_frustum[f].d;

			if (distance < -bSphere.radius)
				return true;
		}

		//Object is inside the volume
		return false;
	}
	catch (...)
	{
		debug.Print("Error in CAMERA::Cull()");
		return true;
	}
}