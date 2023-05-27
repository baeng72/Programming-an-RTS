#include "player.h"		

//Player Shaders
SHADER lighting, teamCol;
D3DXHANDLE worldHandle, viewProjHandle, sunHandle, teamColHandle, vertexCol, mapSizeHandle;
ID3DXLine* selLine = NULL;

void LoadPlayerResources(IDirect3DDevice9* Device)
{
	//Setup shaders
	lighting.Init(Device, "shaders/lighting.vs", VERTEX_SHADER);
	worldHandle = lighting.GetConstant("matW");
	viewProjHandle = lighting.GetConstant("matVP");
	sunHandle = lighting.GetConstant("DirToSun");
	vertexCol = lighting.GetConstant("vertexCol");
	mapSizeHandle = lighting.GetConstant("mapSize");

	teamCol.Init(Device, "shaders/teamCol.ps", PIXEL_SHADER);
	teamColHandle = teamCol.GetConstant("tmCol");

	//Line for unit selection
	D3DXCreateLine(Device, &selLine);
}

void UnloadPlayerResources()
{
	if (selLine != NULL)
		selLine->Release();
	selLine = NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//								PLAYER											//
//////////////////////////////////////////////////////////////////////////////////

PLAYER::PLAYER(int _teamNo, D3DXVECTOR4 _teamCol, INTPOINT startPos, TERRAIN* _terrain, IDirect3DDevice9* _Device)
{
	m_teamNo = _teamNo;
	m_teamColor = _teamCol;
	m_pTerrain = _terrain;
	m_pDevice = _Device;
	m_areaSelect = false;

	if (m_pTerrain == NULL)return;

	//Add a few buildings
	for (int i = 0; i < 3; i++)
	{
		INTPOINT p = FindClosestBuildingLocation(i, startPos);

		if (m_pTerrain->Within(p))
			AddMapObject(i, p, true);
	}

	//Also add a few units
	for (int i = 0; i < 9; i++)
	{
		INTPOINT mp;
		bool ok = false;
		do
		{
			mp.x = rand() % 20 - 10 + startPos.x;
			mp.y = rand() % 20 - 10 + startPos.y;

			MAPTILE* tile = m_pTerrain->GetTile(mp);
			if (tile != NULL)ok = tile->m_walkable && tile->m_pMapObject == NULL;
		} while (!ok);

		AddMapObject(i % 3, mp, false);
	}
}

PLAYER::~PLAYER()
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			delete m_mapObjects[i];
}

void PLAYER::AddMapObject(int type, INTPOINT mp, bool isBuilding)
{
	if (isBuilding)
		m_mapObjects.push_back(new BUILDING(type, m_teamNo, mp, m_pTerrain, true, m_pDevice));
	else m_mapObjects.push_back(new UNIT(type, m_teamNo, mp, m_pTerrain, m_pDevice));
}

void PLAYER::RenderMapObjects(CAMERA& camera)
{
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);

	lighting.SetMatrix(viewProjHandle, camera.GetViewMatrix() * camera.GetProjectionMatrix());

	//Sun direction
	D3DXVECTOR3 sun;
	D3DXVECTOR3 vec = D3DXVECTOR3(0.5f, 1.0f, -0.5);
	D3DXVec3Normalize(&sun, &vec);
	lighting.SetVector3(sunHandle, sun);
	lighting.SetVector3(mapSizeHandle, D3DXVECTOR3(m_pTerrain->m_size.x, m_pTerrain->m_size.y, 0.0f));

	//Team color
	teamCol.SetVector4(teamColHandle, m_teamColor);

	//Fog-Of-War
	m_pDevice->SetTexture(1, m_pTerrain->m_pFogOfWarTexture);

	//Vertex color
	lighting.SetVector4(vertexCol, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));

	lighting.Begin();
	teamCol.Begin();

	//Render units
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			if (!camera.Cull(m_mapObjects[i]->GetBoundingBox()))
			{
				if (m_mapObjects[i]->m_isBuilding)
					lighting.SetMatrix(worldHandle, m_mapObjects[i]->GetWorldMatrix());
				else lighting.SetMatrix(worldHandle, identity);

				m_mapObjects[i]->Render();
			}

	teamCol.End();
	lighting.End();
}

void PLAYER::PaintSelectedMapObjects(CAMERA& camera)
{
	//Paint selected map objects
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			if (!camera.Cull(m_mapObjects[i]->GetBoundingBox()))
				m_mapObjects[i]->PaintSelected();
}

void PLAYER::UpdateMapObjects(float deltaTime)
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			m_mapObjects[i]->Update(deltaTime);
}

INTPOINT PLAYER::FindClosestBuildingLocation(int buildType, INTPOINT mp)
{
	//i = Search Radius
	for (int i = 0; i < 30; i++)
	{
		for (int x = mp.x - i; x <= mp.x + i; x++)
		{
			if (PlaceOk(buildType, INTPOINT(x, mp.y - i), m_pTerrain))return INTPOINT(x, mp.y - i);
			if (PlaceOk(buildType, INTPOINT(x, mp.y + i), m_pTerrain))return INTPOINT(x, mp.y + i);
		}

		for (int y = mp.y - i; y <= mp.y + i; y++)
		{
			if (PlaceOk(buildType, INTPOINT(mp.x - i, y), m_pTerrain))return INTPOINT(mp.x - i, y);
			if (PlaceOk(buildType, INTPOINT(mp.x + i, y), m_pTerrain))return INTPOINT(mp.x + i, y);
		}
	}

	//No good place found
	return INTPOINT(-1, -1);
}

void PLAYER::Select(MOUSE& mouse)
{
	try
	{
		if (mouse.ClickLeft())	// If the mouse button is pressed
		{
			for (int i = 0; i < m_mapObjects.size(); i++)	//Deselect all m_mapObjects
				m_mapObjects[i]->m_selected = false;

			if (!m_areaSelect)		// If no area selection is in progress
			{
				//Find closest m_mapObjects
				int mapObject = -1;
				float bestDist = 100000.0f;

				D3DXMATRIX world;
				D3DXMatrixIdentity(&world);
				m_pDevice->SetTransform(D3DTS_WORLD, &world);
				RAY ray = mouse.GetRay();

				for (int i = 0; i < m_mapObjects.size(); i++)
				{
					float dist = ray.Intersect(m_mapObjects[i]->GetBoundingBox());

					if (dist >= 0.0f && dist < bestDist)
					{
						mapObject = i;
						bestDist = dist;
					}
				}
				if (mapObject > -1)
					m_mapObjects[mapObject]->m_selected = true;
				else
				{
					m_areaSelect = true;		// if no unit if found, 											
					m_startSel = mouse;		// start area selection
				}
			}
			else	//Area Selection in progress
			{
				// Create area rectangle
				INTPOINT p1 = m_startSel, p2 = mouse;
				if (p1.x > p2.x) { int temp = p2.x; p2.x = p1.x; p1.x = temp; }
				if (p1.y > p2.y) { int temp = p2.y; p2.y = p1.y; p1.y = temp; }
				RECT selRect = { p1.x, p1.y, p2.x, p2.y };

				//Draw selection rectangle
				D3DXVECTOR2 box[] = { D3DXVECTOR2(p1.x, p1.y), D3DXVECTOR2(p2.x, p1.y),
									 D3DXVECTOR2(p2.x, p2.y), D3DXVECTOR2(p1.x, p2.y),
									 D3DXVECTOR2(p1.x, p1.y) };

				selLine->SetWidth(1.0f);
				selLine->Begin();
				selLine->Draw(box, 5, 0xffffffff);
				selLine->End();

				//Select any units inside our rectangle
				for (int i = 0; i < m_mapObjects.size(); i++)
					if (m_mapObjects[i] != NULL && !m_mapObjects[i]->m_isBuilding)
					{
						INTPOINT p = GetScreenPos(m_mapObjects[i]->m_position, m_pDevice);
						if (p.inRect(selRect))m_mapObjects[i]->m_selected = true;
					}
			}
		}
		else if (m_areaSelect)		//Stop area selection
			m_areaSelect = false;

	}
	catch (...)
	{
		debug.Print("Error in PLAYER::Select()");
	}
}

void PLAYER::UnitOrders(MOUSE& mouse, bool human)
{
	if (human)
	{
		if (mouse.ClickRight())
		{
			mouse.DisableInput(300);

			for (int i = 0; i < m_mapObjects.size(); i++)
				if (m_mapObjects[i] != NULL)
					if (!m_mapObjects[i]->m_isBuilding && m_mapObjects[i]->m_selected)
					{
						//Cast to UNIT
						UNIT* unit = (UNIT*)m_mapObjects[i];
						unit->Goto(mouse.m_mappos, false, true);
					}
		}
	}
	else
	{
		//Move units randomly
		for (int i = 0; i < m_mapObjects.size(); i++)
			if (m_mapObjects[i] != NULL)
				if (!m_mapObjects[i]->m_isBuilding && rand() % 100 == 0)
				{
					//Cast to UNIT
					UNIT* unit = (UNIT*)m_mapObjects[i];
					INTPOINT p = unit->m_mappos + INTPOINT(rand() % 20 - 10, rand() % 20 - 10);
					p = m_pTerrain->GetClosestFreeTile(p, unit->m_mappos);

					if (!unit->m_moving)
						unit->Goto(p, false, true);
				}
	}
}

INTPOINT PLAYER::GetCenter()
{
	INTPOINT p;

	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			p += m_mapObjects[i]->m_mappos;

	p /= m_mapObjects.size();

	return p;
}

void PLAYER::IsMapObjectsVisible()
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
		{
			if (m_mapObjects[i]->m_isBuilding)
			{
				RECT r = m_mapObjects[i]->GetMapRect(0);
				m_mapObjects[i]->m_visible = false;

				for (int y = r.top; y <= r.bottom && !m_mapObjects[i]->m_visible; y++)
					for (int x = r.left; x <= r.right && !m_mapObjects[i]->m_visible; x++)
						if (m_pTerrain->m_pVisitedTiles[x + y * m_pTerrain->m_size.x])
							m_mapObjects[i]->m_visible = true;
			}
			else
			{
				INTPOINT mp = m_mapObjects[i]->m_mappos;
				m_mapObjects[i]->m_visible = m_pTerrain->m_pVisibleTiles[mp.x + mp.y * m_pTerrain->m_size.x];
			}
		}
}