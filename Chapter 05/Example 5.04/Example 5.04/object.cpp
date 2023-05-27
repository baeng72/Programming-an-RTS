#include "object.h"

std::vector<MESH*> objectMeshes;
ID3DXLine* line = NULL;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* gnome = new MESH("units/warrior_gnome.x", Device);
	objectMeshes.push_back(gnome);

	D3DXCreateLine(Device, &line);

	return S_OK;
}

void UnloadObjectResources()
{
	for (int i = 0; i < objectMeshes.size(); i++)
		objectMeshes[i]->Release();

	objectMeshes.clear();
}

//////////////////////////////////////////////////////////////////////////////
//							OBJECT CLASS									//
//////////////////////////////////////////////////////////////////////////////

OBJECT::OBJECT()
{
	m_type = 0;
}

OBJECT::OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca, std::string _name)
{
	m_type = t;
	m_meshInstance.SetPosition(pos);
	m_meshInstance.SetRotation(rot);
	m_meshInstance.SetScale(sca);
	m_meshInstance.SetMesh(objectMeshes[m_type]);
	m_selected = false;
	m_name = _name;
}

void OBJECT::Render()
{
	m_meshInstance.Render();
}

void OBJECT::PaintSelected()
{
	if (!m_selected)return;

	float radius = 0.6f;
	float height = 2.2f;

	// Create 8 offset points according to the height and radius of a unit
	D3DXVECTOR3 offsets[] = { D3DXVECTOR3(-radius, 0, radius), D3DXVECTOR3(radius, 0, radius),
							 D3DXVECTOR3(-radius, 0, -radius), D3DXVECTOR3(radius, 0, -radius),
							 D3DXVECTOR3(-radius, height, radius), D3DXVECTOR3(radius, height, radius),
							 D3DXVECTOR3(-radius, height, -radius), D3DXVECTOR3(radius, height, -radius) };

	// Find the max and min points of these
	// 8 offests points in screen space
	INTPOINT pmax(-10000, -10000), pmin(10000, 10000);

	for (int i = 0; i < 8; i++)
	{
		D3DXVECTOR3 pos = m_meshInstance.m_pos + offsets[i];
		INTPOINT screenPos = GetScreenPos(pos, m_meshInstance.m_pMesh->m_pDevice);

		if (screenPos.x > pmax.x)pmax.x = screenPos.x;
		if (screenPos.y > pmax.y)pmax.y = screenPos.y;
		if (screenPos.x < pmin.x)pmin.x = screenPos.x;
		if (screenPos.y < pmin.y)pmin.y = screenPos.y;
	}

	RECT scr = { -20, -20, 820, 620 };

	// Check that the max and min point is within our viewport boundaries
	if (pmax.inRect(scr) || pmin.inRect(scr))
	{
		float s = (pmax.x - pmin.x) / 3.0f;
		if ((pmax.y - pmin.y) < (pmax.x - pmin.x))s = (pmax.y - pmin.y) / 3.0f;

		D3DXVECTOR2 corner1[] = { D3DXVECTOR2(pmin.x, pmin.y + s), D3DXVECTOR2(pmin.x, pmin.y), D3DXVECTOR2(pmin.x + s, pmin.y) };
		D3DXVECTOR2 corner2[] = { D3DXVECTOR2(pmax.x - s, pmin.y), D3DXVECTOR2(pmax.x, pmin.y), D3DXVECTOR2(pmax.x, pmin.y + s) };
		D3DXVECTOR2 corner3[] = { D3DXVECTOR2(pmax.x, pmax.y - s), D3DXVECTOR2(pmax.x, pmax.y), D3DXVECTOR2(pmax.x - s, pmax.y) };
		D3DXVECTOR2 corner4[] = { D3DXVECTOR2(pmin.x + s, pmax.y), D3DXVECTOR2(pmin.x, pmax.y), D3DXVECTOR2(pmin.x, pmax.y - s) };

		//Draw the 4 corners
		line->SetWidth(2.0f);
		line->Begin();
		line->Draw(corner1, 3, 0xffffffff);
		line->Draw(corner2, 3, 0xffffffff);
		line->Draw(corner3, 3, 0xffffffff);
		line->Draw(corner4, 3, 0xffffffff);
		line->End();
	}
}