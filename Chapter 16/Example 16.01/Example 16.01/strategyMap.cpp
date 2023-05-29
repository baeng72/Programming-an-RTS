#include "strategyMap.h"

//////////////////////////////////////////////////////////////////////////////////////////
//							AREA														//
//////////////////////////////////////////////////////////////////////////////////////////

AREA::AREA(RECT mArea)
{
	m_mapArea = mArea;
	m_status = STATUS_UNKNOWN;
	m_enemyPresence = 0;
	m_friendlyPresence = 0;
	m_value = 0;
}

void AREA::Update(TERRAIN* terrain, int teamNo)
{
	m_enemyPresence = 0;
	m_friendlyPresence = 0;
	m_value = 0;
	bool visible = false;

	if (terrain == NULL)
		return;

	try
	{
		//Calculate presence in this area
		for (int y = m_mapArea.top; y < m_mapArea.bottom; y++)
			for (int x = m_mapArea.left; x < m_mapArea.right; x++)
			{
				MAPTILE* tile = terrain->GetTile(x, y);

				if (tile != NULL && tile->m_pMapObject != NULL)
				{
					if (tile->m_pMapObject->m_team == teamNo)
					{
						m_friendlyPresence += tile->m_pMapObject->m_hp * tile->m_pMapObject->m_damage;
						visible = true;
					}
					else m_enemyPresence += tile->m_pMapObject->m_hp * tile->m_pMapObject->m_damage;
				}
			}

		//Set area status
		if (visible)
		{
			if (m_enemyPresence > 0 && m_friendlyPresence > 0)m_status = STATUS_CONFLICT;
			else if (m_friendlyPresence > 0)m_status = STATUS_FRIENDLY;
			else if (m_enemyPresence > 0)m_status = STATUS_ENEMY;
		}
	}
	catch (...)
	{
		debug.Print("Error in AREA::Update()");
	}
}

bool AREA::Connected(TERRAIN* terrain, INTPOINT from)
{
	if (terrain == NULL)return false;

	try
	{
		MAPTILE* tile = terrain->GetTile(from);
		if (tile == NULL)return false;

		for (int y = m_mapArea.top; y < m_mapArea.bottom; y++)
			for (int x = m_mapArea.left; x < m_mapArea.right; x++)
			{
				MAPTILE* t = terrain->GetTile(x, y);
				if (t != NULL && t->m_set == tile->m_set)
					return true;
			}
	}
	catch (...)
	{
		debug.Print("Error in AREA::Connected()");
	}

	return false;
}

INTPOINT AREA::GetCenter()
{
	return INTPOINT(m_mapArea.left + (m_mapArea.right - m_mapArea.left) * 0.5f,
		m_mapArea.top + (m_mapArea.bottom - m_mapArea.top) * 0.5f);
}

//////////////////////////////////////////////////////////////////////////////////////////
//							INFLUENCE MAP												//
//////////////////////////////////////////////////////////////////////////////////////////

STRATEGY_MAP::STRATEGY_MAP(TERRAIN* _terrain, PLAYER* _player, INTPOINT numAreas)
{
	m_pTerrain = _terrain;
	m_pPlayer = _player;

	if (m_pTerrain == NULL || m_pPlayer == NULL)return;

	m_size = m_pTerrain->m_size;

	//Create influence map
	m_pInfluenceMap = new int[m_size.x * m_size.y];
	memset(m_pInfluenceMap, 0, sizeof(int) * m_size.x * m_size.y);

	//Create sub areas
	for (int y = 0; y < numAreas.y; y++)
		for (int x = 0; x < numAreas.x; x++)
		{
			RECT r = { x * (m_size.x - 1) / (float)numAreas.x,
					  y * (m_size.y - 1) / (float)numAreas.y,
					(x + 1) * (m_size.x - 1) / (float)numAreas.x,
					(y + 1) * (m_size.y - 1) / (float)numAreas.y };

			m_areas.push_back(AREA(r));
		}
}

STRATEGY_MAP::~STRATEGY_MAP()
{
	if (m_pInfluenceMap != NULL)
		delete[] m_pInfluenceMap;
	m_pInfluenceMap = NULL;
}

void STRATEGY_MAP::Update(std::set<MAPOBJECT*>& enemies)
{
	if (m_pTerrain == NULL || m_pPlayer == NULL)return;

	try
	{
		//Reset influence map
		memset(m_pInfluenceMap, 0, sizeof(int) * m_size.x * m_size.y);

		//Add positive influence from friendly units
		for (int m = 0; m < m_pPlayer->m_mapObjects.size(); m++)
			if (m_pPlayer->m_mapObjects[m] != NULL && !m_pPlayer->m_mapObjects[m]->m_dead)
			{
				MAPOBJECT* mo = m_pPlayer->m_mapObjects[m];
				RECT r = mo->GetMapRect(mo->m_sightRadius);

				if (mo->m_pTerrain != NULL && mo->m_pTerrain->Within(mo->m_mappos))
					for (int y = r.top; y <= r.bottom; y++)
						for (int x = r.left; x <= r.right; x++)
						{
							INTPOINT p(x, y);

							if (m_pTerrain->Within(p))
							{
								float multiplier = (mo->m_sightRadius - mo->m_mappos.Distance(p)) / mo->m_sightRadius;
								if (multiplier < 0.0f)multiplier = 0.0f;
								m_pInfluenceMap[y * m_size.x + x] += mo->m_hp * mo->m_damage * multiplier;
							}
						}
			}

		//Subtract negative influence from enemy units
		std::set<MAPOBJECT*>::iterator i;
		for (i = enemies.begin(); i != enemies.end(); i++)
		{
			if ((*i) != NULL && !(*i)->m_dead)
			{
				MAPOBJECT* mo = (*i);
				RECT r = mo->GetMapRect(mo->m_sightRadius);

				if (mo->m_pTerrain != NULL && mo->m_pTerrain->Within(mo->m_mappos))
					for (int y = r.top; y <= r.bottom; y++)
						for (int x = r.left; x <= r.right; x++)
						{
							INTPOINT p(x, y);

							if (m_pTerrain->Within(p))
							{
								float multiplier = (mo->m_sightRadius - mo->m_mappos.Distance(p)) / mo->m_sightRadius;
								if (multiplier < 0.0f)multiplier = 0.0f;
								m_pInfluenceMap[y * m_size.x + x] -= mo->m_hp * mo->m_damage * multiplier;
							}
						}
			}
		}

		//Update Map Areas
		for (int i = 0; i < m_areas.size(); i++)
			m_areas[i].Update(m_pTerrain, m_pPlayer->m_teamNo);
	}
	catch (...)
	{
		debug.Print("Error in STRATEGY_MAP::Update()");
	}
}


AREA* STRATEGY_MAP::GetScoutArea(INTPOINT from)
{
	int index = -1;
	float distance = 100000.0f;

	//Find closest unexplored area
	for (int i = 0; i < m_areas.size(); i++)
	{
		INTPOINT areaCenter = m_areas[i].GetCenter();

		if (m_areas[i].m_status == STATUS_UNKNOWN && m_areas[i].Connected(m_pTerrain, from))
		{
			float d = from.Distance(areaCenter) + m_pPlayer->m_teamStartLocation.Distance(areaCenter) * 0.6f;

			if (d < distance)
			{
				index = i;
				distance = d;
			}
		}
	}

	if (index >= 0)
		return &m_areas[index];
	else return GetRandomArea();
}

AREA* STRATEGY_MAP::GetAttackArea(INTPOINT from)
{
	int index = -1;
	float distance = 100000.0f;

	//Find closest enemy occupied area
	for (int i = 0; i < m_areas.size(); i++)
	{
		INTPOINT areaCenter = m_areas[i].GetCenter();

		if (m_areas[i].m_status == STATUS_ENEMY || m_areas[i].m_status == STATUS_CONFLICT)
		{
			float d = from.Distance(areaCenter) + m_pPlayer->m_teamStartLocation.Distance(areaCenter) * 0.6f;

			if (d < distance)
			{
				index = i;
				distance = d;
			}
		}
	}

	if (index >= 0)
		return &m_areas[index];
	else return GetRandomArea();
}

AREA* STRATEGY_MAP::GetRandomArea()
{
	return &m_areas[rand() % m_areas.size()];
}