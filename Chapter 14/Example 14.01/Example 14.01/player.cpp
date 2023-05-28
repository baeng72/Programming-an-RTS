#include "player.h"		

//Player Shaders
SHADER lighting, teamCol;
D3DXHANDLE worldHandle, viewProjHandle, sunHandle, teamColHandle, vertexCol, mapSizeHandle;
ID3DXLine* selLine = NULL;
ID3DXSprite* menuSprite = NULL;
IDirect3DTexture9* uiTexture = NULL;
ID3DXFont* fontMoney = NULL;

extern D3DRECT SetRect(long x1, long y1, long x2, long y2);

void LoadPlayerResources(IDirect3DDevice9* m_pDevice)
{
	//Setup shaders
	lighting.Init(m_pDevice, "shaders/lighting.vs", VERTEX_SHADER);
	worldHandle = lighting.GetConstant("matW");
	viewProjHandle = lighting.GetConstant("matVP");
	sunHandle = lighting.GetConstant("DirToSun");
	vertexCol = lighting.GetConstant("vertexCol");
	mapSizeHandle = lighting.GetConstant("mapSize");

	teamCol.Init(m_pDevice, "shaders/teamCol.ps", PIXEL_SHADER);
	teamColHandle = teamCol.GetConstant("tmCol");

	//Line for unit selection
	D3DXCreateLine(m_pDevice, &selLine);

	D3DXCreateSprite(m_pDevice, &menuSprite);

	D3DXCreateTextureFromFile(m_pDevice, "textures/ui.dds", &uiTexture);

	D3DXCreateFont(m_pDevice, 20, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial Black", &fontMoney);
}

void UnloadPlayerResources()
{
	if (selLine != NULL)
		selLine->Release();
	selLine = NULL;

	if (menuSprite != NULL)
		menuSprite->Release();
	menuSprite = NULL;

	if (uiTexture != NULL)
		uiTexture->Release();
	uiTexture = NULL;
}

float GetCost(int type, bool m_isBuilding)
{
	if (m_isBuilding)
	{
		if (type == TOWNHALL)return 25.0f;
		if (type == BARRACKS)return 20.0f;
		if (type == TOWER)return 30.0f;
	}
	else
	{
		if (type == WORKER)return 7.0f;
		if (type == SOLDIER)return 12.0f;
		if (type == MAGICIAN)return 15.0f;
	}

	return 100.0f;
}

//////////////////////////////////////////////////////////////////////////////////
//								PLAYER											//
//////////////////////////////////////////////////////////////////////////////////

PLAYER::PLAYER(int _teamNo, int _controller, D3DXVECTOR4 _teamCol, INTPOINT startPos, TERRAIN* _terrain, IDirect3DDevice9* _Device)
{
	m_teamNo = _teamNo;
	m_teamColor = _teamCol;
	m_pTerrain = _terrain;
	m_pDevice = _Device;
	m_areaSelect = m_placeBuilding = false;
	m_nextUnitUpdate = 0.5f;
	m_unitUpdateIndex = 0;
	m_time = 0.0f;
	m_controller = _controller;
	m_pAi = NULL;
	money = 0.0f;
	unitLimit = 15;
	m_teamStartLocation = startPos;
	m_numKills = 0;

	if (m_pTerrain == NULL)return;

	//Add a few buildings
	for (int i = 0; i < 0; i++)
	{
		INTPOINT p = FindClosestBuildingLocation(i, startPos);

		if (m_pTerrain->Within(p))
			AddMapObject(i, p, true, true);
	}

	//startPos = GetCenter();

	//Also add a few units
	for (int i = 0; i < 1; i++)
	{
		INTPOINT mp;
		bool ok = false;
		do
		{
			mp.x = rand() % 10 - 5 + startPos.x;
			mp.y = rand() % 10 - 5 + startPos.y;

			MAPTILE* tile = m_pTerrain->GetTile(mp);
			if (tile != NULL)ok = tile->m_walkable && tile->m_pMapObject == NULL;
		} while (!ok);

		AddMapObject(i % 3, mp, false, true);
	}

	if (m_controller == COMPUTER)
	{
		m_pAi = new MASTERAI(this, m_pTerrain);
	}
}

PLAYER::~PLAYER()
{
	if (m_pAi != NULL)
		delete m_pAi;

	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			delete m_mapObjects[i];
}

MAPOBJECT* PLAYER::AddMapObject(int type, INTPOINT mp, bool m_isBuilding, bool finished)
{
	if (m_isBuilding)
		m_mapObjects.push_back(new BUILDING(type, m_teamNo, finished, mp, m_pTerrain, this, true, m_pDevice));
	else m_mapObjects.push_back(new UNIT(type, m_teamNo, mp, m_pTerrain, this, m_pDevice));

	return m_mapObjects[m_mapObjects.size() - 1];
}

void PLAYER::RemoveMapObject(MAPOBJECT* mapObject)
{
	try
	{
		std::vector<MAPOBJECT*>::iterator i;
		for (i = m_mapObjects.begin(); i != m_mapObjects.end(); i++)
		{
			if ((*i) == mapObject)
			{
				m_mapObjects.erase(i);
				break;
			}
		}
	}
	catch (...)
	{
		debug.Print("Error in PLAYER::RemoveMapObject()");
	}
}

void PLAYER::RenderMapObjects(CAMERA& camera)
{
	try
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

		//Render building fires
		for (int i = 0; i < m_mapObjects.size(); i++)
			if (m_mapObjects[i] != NULL && m_mapObjects[i]->m_isBuilding)
			{
				BUILDING* build = (BUILDING*)m_mapObjects[i];
				build->RenderFires();
			}
	}
	catch (...)
	{
		debug.Print("Error in PLAYER::RenderMapObjects()");
	}
}

void PLAYER::PaintSelectedMapObjects(CAMERA& camera)
{
	//Paint m_selected map objects
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			if (!camera.Cull(m_mapObjects[i]->GetBoundingBox()))
				m_mapObjects[i]->PaintSelected(m_time * D3DX_PI * 2.0f);
}

void PLAYER::UpdateMapObjects(float deltaTime)
{
	try
	{
		//Player time
		m_time += deltaTime * 0.05f;
		if (m_time > 1.0f)m_time -= 1.0f;
		money += deltaTime;
		if (money > 100.0f)money = 100.0f;

		//Update map objects
		std::vector<MAPOBJECT*> toDelete;
		//std::vector<MAPOBJECT*>::iterator i = m_mapObjects.begin();
		//while (i != m_mapObjects.end())
		int i = 0;
		for (i = 0; i < m_mapObjects.size(); i++) {
			auto mo = (UNIT*)m_mapObjects[i];
			if (mo) {
				if (mo->isDead())
					toDelete.push_back(mo);
				else
					mo->Update(deltaTime);

			}
		}
		//for(i=m_mapObjects.begin();i!=m_mapObjects.end();i++)
		//{
		//	if ((*i)->isDead())
		//	{
		//		toDelete.push_back(*i);
		//		//if ((*i) != NULL)delete (*i);
		//		//m_mapObjects.erase(i);
		//		
		//	}
		//	else
		//	{
		//		(*i)->Update(deltaTime);
		//		//i++;
		//	}
		//}
		for (auto p : toDelete) {

			m_mapObjects.erase(std::find(m_mapObjects.begin(), m_mapObjects.end(), p));
			delete p;
		}

		//Call UnitAI() for each mapObject twice a second
		m_nextUnitUpdate -= deltaTime;
		if (m_nextUnitUpdate < 0.0f && !m_mapObjects.empty())
		{
			m_unitUpdateIndex++;
			if (m_unitUpdateIndex >= m_mapObjects.size())
				m_unitUpdateIndex = 0;

			m_nextUnitUpdate = 0.5f / (float)m_mapObjects.size();

			if (!m_mapObjects[m_unitUpdateIndex]->m_isBuilding)
			{
				UNIT* unit = (UNIT*)m_mapObjects[m_unitUpdateIndex];
				unit->UnitAI(false);
			}
		}

		//Master AI
		if (m_controller == COMPUTER && m_pAi != NULL)
			m_pAi->Update(deltaTime);
	}
	catch (...)
	{
		debug.Print("Error in PLAYER::UpdateMapObjects()");
	}
}

INTPOINT PLAYER::FindClosestBuildingLocation(int buildType, INTPOINT mp)
{
	try
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
	}
	catch (...)
	{
		debug.Print("Error in PLAYER::FindClosestBuildingLocation()");
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
			//If user pressed in the menu or minimap don't start the selection process
			RECT minimap = { 611, 9, 791, 189 };
			RECT UI = { 650, 430, 800, 600 };
			if (mouse.inRect(minimap))return;
			if (mouse.inRect(UI))return;

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
					if (m_mapObjects[i] != NULL && !m_mapObjects[i]->m_dead)
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
					if (m_mapObjects[i] != NULL && !m_mapObjects[i]->m_isBuilding && !m_mapObjects[i]->m_dead)
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

void PLAYER::UnitOrders(MOUSE& mouse, std::vector<PLAYER*>& players, CAMERA& camera)
{
	if (m_placeBuilding)return;

	RECT minimap = { 611, 9, 791, 189 };
	RECT UI = { 650, 430, 800, 600 };

	if (mouse.ClickRight())
	{
		if (mouse.inRect(minimap))return;
		if (mouse.inRect(UI))return;

		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);
		m_pDevice->SetTransform(D3DTS_WORLD, &identity);
		D3DXMATRIX view = camera.GetViewMatrix();
		m_pDevice->SetTransform(D3DTS_VIEW, &view);
		D3DXMATRIX proj = camera.GetProjectionMatrix();
		m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

		RAY mRay = mouse.GetRay();
		mouse.DisableInput(300);

		MAPOBJECT* enemyTarget = NULL;
		float bestDist = 100000.0f;

		//See what enemy target the ray intersects
		for (int p = 0; p < players.size(); p++)
			if (players[p] != NULL && players[p]->m_teamNo != m_teamNo)
				for (int m = 0; m < players[p]->m_mapObjects.size(); m++)
					if (players[p]->m_mapObjects[m] != NULL)
					{
						float dist = mRay.Intersect(players[p]->m_mapObjects[m]->GetBoundingBox());

						if (dist > 0.0f && dist < bestDist)
						{
							enemyTarget = players[p]->m_mapObjects[m];
							bestDist = dist;
						}
					}

		//Give orders
		for (int i = 0; i < m_mapObjects.size(); i++)
			if (m_mapObjects[i] != NULL)
				if (!m_mapObjects[i]->m_isBuilding && m_mapObjects[i]->m_selected)
				{
					//Cast to UNIT
					UNIT* unit = (UNIT*)m_mapObjects[i];

					if (enemyTarget == NULL)
					{
						unit->Goto(mouse.m_mappos, false, true, STATE_MOVING);
					}
					else
					{
						unit->Attack(enemyTarget);
					}
				}
	}
}

BUILDING* PLAYER::GetAvailableBuilding(int type)
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL && m_mapObjects[i]->m_type == type &&
			m_mapObjects[i]->m_isBuilding && !m_mapObjects[i]->m_dead)
		{
			BUILDING* building = (BUILDING*)m_mapObjects[i];
			if (building->m_buildProgress >= 1.0f && !building->m_training)
				return building;
		}

	return NULL;
}

UNIT* PLAYER::GetAvailableUnit(int type)
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL && m_mapObjects[i]->m_type == type &&
			!m_mapObjects[i]->m_isBuilding && !m_mapObjects[i]->m_dead)
		{
			UNIT* unit = (UNIT*)m_mapObjects[i];
			if (unit->m_state == STATE_IDLE)
				return unit;
		}

	return NULL;
}

INTPOINT PLAYER::GetCenter()
{
	INTPOINT p;

	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL)
			p += m_mapObjects[i]->m_mappos;

	if (m_mapObjects.size() > 0)
		p /= m_mapObjects.size();

	return p;
}

RECT PLAYER::GetBaseArea()
{
	RECT r = { 10000, 10000, -10000, -10000 };

	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i] != NULL && m_mapObjects[i]->m_isBuilding)
		{
			RECT mr = m_mapObjects[i]->GetMapRect(5);

			if (mr.left < r.left)r.left = mr.left;
			if (mr.right > r.right)r.right = mr.right;
			if (mr.top < r.top)r.top = mr.top;
			if (mr.bottom > r.bottom)r.bottom = mr.bottom;
		}

	return r;
}

void PLAYER::IsMapObjectsVisible()
{
	try
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
	catch (...)
	{
		debug.Print("Error in PLAYER::IsMapObjectsVisible()");
	}
}

bool MenuButton(int src, int dest, bool active, MOUSE& mouse)
{
	RECT srcRects[] = { {0, 0, 55, 70}, {0, 70, 55, 140}, {0, 140, 55, 210},
					   {165, 0, 220, 70}, {165, 70, 220, 140}, {165, 140, 220, 210} };

	RECT destRects[] = { {665, 440, 715, 510}, {730, 440, 785, 510},
						{665, 520, 715, 590}, {730, 520, 785, 590} };

	bool over = mouse.Over(destRects[dest]);
	bool press = false;

	if (!active)
	{
		srcRects[src].left += 110;
		srcRects[src].right += 110;
	}
	else
	{
		if (mouse.PressInRect(destRects[dest]))
		{
			mouse.DisableInput(300);
			press = true;
		}

		if (over) { srcRects[src].left += 55; srcRects[src].right += 55; }
	}
	D3DXVECTOR3 vec = D3DXVECTOR3(destRects[dest].left, destRects[dest].top, 0.0f);
	menuSprite->Draw(uiTexture, &srcRects[src], NULL, &vec, 0xffffffff);

	return active && press;
}

void PLAYER::Menu(MOUSE& mouse)
{
	if (m_placeBuilding)return;

	D3DRECT r = SetRect(660, 440, 790, 590);
	m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

	RECT uiSrc = { 0, 210, 150, 380 };
	RECT moneySrc = { 150, 210, 400, 245 };

	//find m_selected unit or building
	MAPOBJECT* m_selected = NULL;
	int numSelected = 0, numUnits = 0;
	int numBuildings[] = { 0, 0, 0 };

	for (int i = 0; i < m_mapObjects.size(); i++)
	{
		if (m_mapObjects[i]->m_selected)
		{
			numSelected++;
			m_selected = m_mapObjects[i];
		}

		//Count buildings & units
		if (!m_mapObjects[i]->m_dead)
		{
			if (m_mapObjects[i]->m_isBuilding)
				numBuildings[m_mapObjects[i]->m_type]++;
			else numUnits++;
		}
	}

	//User interface menu...
	menuSprite->Begin(D3DXSPRITE_ALPHABLEND);

	//Unit / Building menu
	if (numSelected == 1)
	{
		if (m_selected->m_isBuilding)
		{
			BUILDING* building = (BUILDING*)m_selected;

			if (!building->m_training && building->m_buildProgress >= 1.0f)
			{
				if (building->m_type == TOWNHALL)
					if (MenuButton(WORKER, 0, GetCost(WORKER, false) < money && numUnits < unitLimit, mouse))
						building->TrainUnit(WORKER);

				if (building->m_type == BARRACKS)
					if (MenuButton(SOLDIER, 0, GetCost(SOLDIER, false) < money && numUnits < unitLimit, mouse))
						building->TrainUnit(SOLDIER);

				if (building->m_type == TOWER)
					if (MenuButton(MAGICIAN, 0, GetCost(MAGICIAN, false) < money && numUnits < unitLimit, mouse))
						building->TrainUnit(MAGICIAN);
			}
		}
		else
		{
			m_pSelectedUnit = (UNIT*)m_selected;

			if (m_pSelectedUnit->m_type == WORKER)
			{
				if (MenuButton(TOWNHALL + 3, 0, GetCost(TOWNHALL, true) < money, mouse)) { m_buildingToPlace = TOWNHALL; m_placeBuilding = true; }
				if (MenuButton(BARRACKS + 3, 1, numBuildings[0] > 0 && GetCost(BARRACKS, true) < money, mouse)) { m_buildingToPlace = BARRACKS; m_placeBuilding = true; }
				if (MenuButton(TOWER + 3, 2, numBuildings[0] > 0 && numBuildings[1] > 0 && GetCost(TOWER, true) < money, mouse)) { m_buildingToPlace = TOWER; m_placeBuilding = true; }
			}
		}
	}
	D3DXVECTOR3 vec = D3DXVECTOR3(650.0f, 430.0f, 0.0f);
	menuSprite->Draw(uiTexture, &uiSrc, NULL, &vec, 0xffffffff);

	//Draw player money counter
	vec = D3DXVECTOR3(10.0f, -2.0f, 0.0f);
	menuSprite->Draw(uiTexture, &moneySrc, NULL, &vec, 0xffffffff);

	char num[10];
	//itoa(money * 10, num, 10);
	_itoa_s(money * 10, num, sizeof(num), 10);
	RECT rc[] = { {80, 2, 0, 0}, {162, 2, 0, 0} };
	fontMoney->DrawText(NULL, num, -1, &rc[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

	//std::string u = itoa(numUnits, num, 10) + std::string(" / ") + itoa(unitLimit, num, 10);
	std::string u;
	_itoa_s(numUnits, num, sizeof(num), 10);
	u = num + std::string(" / ");
	_itoa_s(unitLimit, num, sizeof(num), 10);
	u += num;
	fontMoney->DrawText(NULL, u.c_str(), -1, &rc[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

	menuSprite->End();
}

void PLAYER::PlaceBuilding(MOUSE& mouse, CAMERA& camera)
{
	if (!m_placeBuilding)return;

	//Create building to place...
	BUILDING* building = new BUILDING(m_buildingToPlace, m_teamNo, true, mouse.m_mappos, m_pTerrain, this, false, m_pDevice);
	building->m_visible = true;

	//Mouse input
	if (mouse.ClickLeft())
	{
		mouse.DisableInput(300);

		if (PlaceOk(m_buildingToPlace, mouse.m_mappos, m_pTerrain))
		{
			m_pSelectedUnit->ConstructBuilding(m_buildingToPlace, mouse.m_mappos);
			m_placeBuilding = false;
		}
	}
	else if (mouse.ClickRight())
	{
		m_placeBuilding = false;
		mouse.DisableInput(300);
	}

	//Render building to place
	lighting.SetMatrix(viewProjHandle, camera.GetViewMatrix() * camera.GetProjectionMatrix());

	D3DXVECTOR3 sun;
	D3DXVECTOR3 vec = D3DXVECTOR3(0.5f, 1.0f, -0.5);
	D3DXVec3Normalize(&sun, &vec);
	lighting.SetVector3(sunHandle, sun);
	teamCol.SetVector4(teamColHandle, m_teamColor);

	//Fog-Of-War
	m_pDevice->SetTexture(1, m_pTerrain->m_pFogOfWarTexture);

	lighting.Begin();
	teamCol.Begin();

	//Green if place ok, red otherwise
	if (PlaceOk(m_buildingToPlace, mouse.m_mappos, m_pTerrain)) {
		lighting.SetVector4(vertexCol, D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else lighting.SetVector4(vertexCol, D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f));
	lighting.SetMatrix(worldHandle, building->GetWorldMatrix());
	building->Render();

	teamCol.End();
	lighting.End();

	delete building;
}

bool PLAYER::HasMapObject(int type, bool m_isBuilding)
{
	for (int i = 0; i < m_mapObjects.size(); i++)
		if (m_mapObjects[i]->m_isBuilding == m_isBuilding)
			if (m_mapObjects[i]->m_type == type)
				return true;

	return false;
}
