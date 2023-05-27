#include "mapObject.h"

//Line interface to draw selected units/buildings with
ID3DXLine* line = NULL;

void LoadMapObjectResources(IDirect3DDevice9* Device)
{
	D3DXCreateLine(Device, &line);
}

void UnloadMapObjectResources()
{
	if (line)line->Release();
	line = NULL;
}

INTPOINT GetScreenPos(D3DXVECTOR3 pos, IDirect3DDevice9* Device)
{
	D3DXVECTOR3 screenPos;
	D3DVIEWPORT9 Viewport;
	D3DXMATRIX Projection, View, World;

	Device->GetViewport(&Viewport);
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	D3DXMatrixIdentity(&World);
	D3DXVec3Project(&screenPos, &pos, &Viewport, &Projection, &View, &World);

	return INTPOINT(screenPos.x, screenPos.y);
}


//////////////////////////////////////////////////////////////////////////////////
//								MapObject										//
//////////////////////////////////////////////////////////////////////////////////

MAPOBJECT::MAPOBJECT()
{
	//Sets all variables to 0, NULL or False
	m_isBuilding = false;
	m_pTerrain = NULL;
	m_hp = m_hpMax = 0;
	m_range = m_damage = 0;
	m_sightRadius = 0.0f;
	m_team = m_type = 0;
	m_selected = m_dead = false;
	m_pTarget = NULL;
	m_pDevice = NULL;
}

RECT MAPOBJECT::GetMapRect(int border)
{
	RECT mr = { m_mappos.x - border,
			   m_mappos.y - border,
			   m_mappos.x + m_mapsize.x + border,
			   m_mappos.y + m_mapsize.y + border };

	return mr;
}

void MAPOBJECT::PaintSelected()
{
	if (!m_selected || m_pDevice == NULL)return;

	BBOX bbox = GetBoundingBox();	//Bounding box in world space

	// Create 8 points according to the corners of the bounding box
	D3DXVECTOR3 corners[] = { D3DXVECTOR3(bbox.max.x, bbox.max.y, bbox.max.z),
							 D3DXVECTOR3(bbox.max.x, bbox.max.y, bbox.min.z),
							 D3DXVECTOR3(bbox.max.x, bbox.min.y, bbox.max.z),
							 D3DXVECTOR3(bbox.max.x, bbox.min.y, bbox.min.z),
							 D3DXVECTOR3(bbox.min.x, bbox.max.y, bbox.max.z),
							 D3DXVECTOR3(bbox.min.x, bbox.max.y, bbox.min.z),
							 D3DXVECTOR3(bbox.min.x, bbox.min.y, bbox.max.z),
							 D3DXVECTOR3(bbox.min.x, bbox.min.y, bbox.min.z) };

	// Find the max and min points of these
	// 8 offests points in screen space
	INTPOINT pmax(-10000, -10000), pmin(10000, 10000);

	for (int i = 0; i < 8; i++)
	{
		INTPOINT screenPos = GetScreenPos(corners[i], m_pDevice);

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
		if (line != NULL)
		{
			line->SetWidth(2.0f);
			line->Begin();
			line->Draw(corner1, 3, 0xffffffff);
			line->Draw(corner2, 3, 0xffffffff);
			line->Draw(corner3, 3, 0xffffffff);
			line->Draw(corner4, 3, 0xffffffff);
			line->End();
		}
	}
}