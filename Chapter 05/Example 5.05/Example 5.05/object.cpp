#include "object.h"

std::vector<MESH*> objectMeshes;
ID3DXLine* line = NULL;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* f1a = new MESH("objects/f1a.x", Device);
	objectMeshes.push_back(f1a);

	MESH* f1b = new MESH("objects/f1b.x", Device);
	objectMeshes.push_back(f1b);

	D3DXCreateLine(Device, &line);

	return S_OK;
}

void UnloadObjectResources()
{
	for (int i = 0; i < objectMeshes.size(); i++)
		objectMeshes[i]->Release();

	objectMeshes.clear();

	line->Release();
}

//////////////////////////////////////////////////////////////////////////////
//							OBJECT CLASS									//
//////////////////////////////////////////////////////////////////////////////

OBJECT::OBJECT()
{
	m_type = 0;
}

OBJECT::OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, float off)
{
	m_type = t;
	m_meshInstance.SetPosition(pos);
	m_meshInstance.SetRotation(rot);
	m_meshInstance.SetScale(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	m_meshInstance.SetMesh(objectMeshes[m_type]);

	m_prc = 0.0f;
	m_speed = 30.0f;
	m_activeWP = 0;
	m_nextWP = m_activeWP + 1;
	m_offset = off;

	//Add cameras
	m_activeCam = 0;
	m_cameras.push_back(CAMERA());
	m_cameras.push_back(CAMERA());
	m_cameras.push_back(CAMERA());

	for (int c = 0; c < m_cameras.size(); c++)
		m_cameras[c].Init(m_meshInstance.m_pMesh->m_pDevice);
}

D3DXVECTOR2 wayPoints[] = { D3DXVECTOR2(0.05f, -1.054f), D3DXVECTOR2(76.839f, -3.446f), D3DXVECTOR2(85.238f, -4.87f), D3DXVECTOR2(92.321f, -9.393f),
						   D3DXVECTOR2(95.897f, -16.917f), D3DXVECTOR2(95.33f, -25.371f), D3DXVECTOR2(93.056f, -33.568f), D3DXVECTOR2(66.702f, -105.733f),
						   D3DXVECTOR2(61.049f, -112.011f), D3DXVECTOR2(54.085f, -116.594f), D3DXVECTOR2(46.065f, -119.516f), D3DXVECTOR2(37.617f, -120.27f),
						   D3DXVECTOR2(-64.766f, -117.602f), D3DXVECTOR2(-73.053f, -115.573f), D3DXVECTOR2(-80.386f, -111.628f), D3DXVECTOR2(-86.758f, -105.98f),
						   D3DXVECTOR2(-130.339f, -53.405f), D3DXVECTOR2(-135.042f, -46.442f), D3DXVECTOR2(-137.235f, -38.218f), D3DXVECTOR2(-136.028f, -29.767f),
						   D3DXVECTOR2(-131.556f, -22.549f), D3DXVECTOR2(-124.52f, -18.613f), D3DXVECTOR2(-116.265f, -16.981f), D3DXVECTOR2(-107.82f, -18.225f),
						   D3DXVECTOR2(-100.507f, -22.037f), D3DXVECTOR2(-94.091f, -27.636f), D3DXVECTOR2(-60.762f, -66.526f), D3DXVECTOR2(-53.834f, -71.231f),
						   D3DXVECTOR2(-45.89f, -73.977f), D3DXVECTOR2(13.784f, -75.918f), D3DXVECTOR2(22.103f, -74.003f), D3DXVECTOR2(28.424f, -68.345f),
						   D3DXVECTOR2(32.008f, -60.721f), D3DXVECTOR2(32.969f, -52.331f), D3DXVECTOR2(31.777f, -44.109f), D3DXVECTOR2(28.054f, -36.428f),
						   D3DXVECTOR2(21.441f, -31.122f), D3DXVECTOR2(13.529f, -28.57f), D3DXVECTOR2(5.153f, -28.072f), D3DXVECTOR2(-3.018f, -30.543f),
						   D3DXVECTOR2(-7.975f, -37.213f), D3DXVECTOR2(-12.147f, -44.651f), D3DXVECTOR2(-17.197f, -50.976f), D3DXVECTOR2(-25.131f, -54.126f),
						   D3DXVECTOR2(-33.395f, -53.507f), D3DXVECTOR2(-41.168f, -50.853f), D3DXVECTOR2(-46.317f, -44.045f), D3DXVECTOR2(-47.582f, -35.874f),
						   D3DXVECTOR2(-47.26f, -27.425f), D3DXVECTOR2(-44.497f, -19.349f), D3DXVECTOR2(-39.651f, -12.716f), D3DXVECTOR2(-32.743f, -7.802f),
						   D3DXVECTOR2(-24.947f, -4.325f), D3DXVECTOR2(-17.018f, -1.44f), D3DXVECTOR2(-8.484f, -1.247f) };

D3DXVECTOR3 trackCenter = D3DXVECTOR3(-10.0f, 0.0f, -60.0f);

void OBJECT::Update(float deltaTime)
{
	D3DXVECTOR2 vec = (wayPoints[m_activeWP] - wayPoints[m_nextWP]);
	float distance = D3DXVec2Length(&vec);
	m_prc += (deltaTime * m_speed) / distance;

	if (m_prc >= 1.0f)
	{
		m_prc -= 1.0f;
		m_activeWP++;
		if (m_activeWP > 54)m_activeWP = 0;

		m_nextWP = m_activeWP + 1;
		if (m_nextWP > 54)m_nextWP = 0;
	}

	D3DXVECTOR3 a = D3DXVECTOR3(wayPoints[m_activeWP].x, 0.0f, wayPoints[m_activeWP].y);
	D3DXVECTOR3 b = D3DXVECTOR3(wayPoints[m_nextWP].x, 0.0f, wayPoints[m_nextWP].y);

	//Linear interpolation between the waypoints
	m_meshInstance.m_pos = a - (a * m_prc) + (b * m_prc);
	m_direction = b - a;
	D3DXVec3Normalize(&m_direction, &m_direction);

	//Offset car from track
	D3DXVECTOR3 dirCenter = m_meshInstance.m_pos - trackCenter;
	D3DXVec3Normalize(&dirCenter, &dirCenter);
	m_meshInstance.m_pos += dirCenter * m_offset;

	//Calculate the rotation angle
	float angle = 0.0f;
	if (wayPoints[m_activeWP].x - wayPoints[m_nextWP].x != 0.0f)
	{
		float f1 = wayPoints[m_nextWP].y - wayPoints[m_activeWP].y;
		float f2 = wayPoints[m_nextWP].x - wayPoints[m_activeWP].x;
		angle = atan(f1 / -f2);
		if (f2 < 0.0f)angle -= D3DX_PI;
	}

	m_meshInstance.m_rot.y = angle;
}

void OBJECT::UpdateCameras()
{
	//Camera 1:		Driver's Head
	m_cameras[0].m_eye = m_meshInstance.m_pos + m_direction * -4.0f + D3DXVECTOR3(0.0f, 1.5f, -1.0f);
	m_cameras[0].m_focus = m_meshInstance.m_pos + m_direction * 1.0f + D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_cameras[0].m_fov = D3DX_PI * 0.3f;

	//Camera 2:		Track Center
	m_cameras[1].m_eye = trackCenter + D3DXVECTOR3(0.0f, 50.0f, 0.0f);
	m_cameras[1].m_focus = m_meshInstance.m_pos;
	m_cameras[1].m_fov = D3DX_PI * 0.1f;

	//Camera 3:		In front of the car
	m_cameras[2].m_eye = m_meshInstance.m_pos + m_direction * 5.0f + D3DXVECTOR3(0.0f, 0.3f, 0.0f);
	m_cameras[2].m_focus = m_meshInstance.m_pos + D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_cameras[2].m_fov = D3DX_PI * 0.2f;

	D3DXMATRIX world = m_cameras[m_activeCam].GetViewMatrix();
	m_meshInstance.m_pMesh->m_pDevice->SetTransform(D3DTS_VIEW, &world);
	D3DXMATRIX proj = m_cameras[m_activeCam].GetProjectionMatrix();
	m_meshInstance.m_pMesh->m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);
}

void OBJECT::Render()
{
	m_meshInstance.Render();
}