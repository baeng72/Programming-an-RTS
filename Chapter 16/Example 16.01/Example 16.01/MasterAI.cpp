#include "masterAI.h"
#include "strategyMap.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//								MASTERAI													//
//////////////////////////////////////////////////////////////////////////////////////////////

MASTERAI::MASTERAI(PLAYER* _player, TERRAIN* _terrain) : m_unitPool(this)
{
	m_pPlayer = _player;
	m_pTerrain = _terrain;
	m_nextGroupUpdate = 1.0f;
	m_nextUpdate = 5.0f + m_pPlayer->m_teamNo;

	INTPOINT center = m_pPlayer->GetCenter();
	RECT r = { center.x - 10, center.y - 10, center.x + 10, center.y + 10 };
	m_base = r;

	//Create influence map
	m_pStrategyMap = new STRATEGY_MAP(m_pTerrain, m_pPlayer, INTPOINT(10, 10));
}

MASTERAI::~MASTERAI()
{
	//Delete all groups
	for (int i = 0; i < m_groups.size(); i++)
		if (m_groups[i] != NULL)
			delete m_groups[i];
	m_groups.clear();

	//Release strategy map
	if (m_pStrategyMap != NULL)
		delete m_pStrategyMap;
}

void MASTERAI::Update(float deltaTime)
{
	if (m_pPlayer == NULL || m_pTerrain == NULL)return;

	//Group AI update
	m_nextGroupUpdate -= deltaTime;
	if (m_nextGroupUpdate < 0.0f)
	{
		m_nextGroupUpdate = 3.0f;
		HandleGroups();
	}

	//Master AI Update
	m_nextUpdate -= deltaTime;
	if (m_nextUpdate < 0.0f)
	{
		m_nextUpdate = 10.0f;
		MasterAI();
	}
}

void MASTERAI::HandleGroups()
{
	//Add unassigned units/buildings to the unit pool
	for (int m = 0; m < m_pPlayer->m_mapObjects.size(); m++)
		if (m_pPlayer->m_mapObjects[m] != NULL && !m_pPlayer->m_mapObjects[m]->m_dead)
			if (m_pPlayer->m_mapObjects[m]->m_pGroup == NULL)
				m_unitPool.AddMember(m_pPlayer->m_mapObjects[m]);

	//Update unit Pool
	m_base = m_pPlayer->GetBaseArea();
	m_unitPool.SetTask(TASK_DEFEND_LOCATION, &m_base);
	m_unitPool.GroupAI();

	//Update other groups
	std::vector<GROUPAI*>::iterator i;
	for (i = m_groups.begin(); i != m_groups.end();)
	{
		if ((*i) == NULL || (*i)->isDead())
		{
			delete (*i);
			m_groups.erase(i);
		}
		else
		{
			(*i)->GroupAI();
			i++;
		}
	}
}

void MASTERAI::MasterAI()
{
	//Update Influence map
	if (m_pStrategyMap != NULL)
		m_pStrategyMap->Update(m_visibleEnemies);

	m_visibleEnemies.clear();

	//Build buildings and train units
	BuildManager();

	//Command units
	StrategicManager();
}

void MASTERAI::BuildManager()
{
	//Count number of buildings and units
	int totNumUnits = 0;
	int numUnits[3] = { 0, 0, 0 };
	int numBuildings[3] = { 0, 0, 0 };

	for (int i = 0; i < m_pPlayer->m_mapObjects.size(); i++)
		if (m_pPlayer->m_mapObjects[i] != NULL)
		{
			if (m_pPlayer->m_mapObjects[i]->m_isBuilding)
				numBuildings[m_pPlayer->m_mapObjects[i]->m_type]++;
			else
			{
				numUnits[m_pPlayer->m_mapObjects[i]->m_type]++;
				totNumUnits++;
			}
		}

	//Cant train more units than the unit limit
	if (totNumUnits >= m_pPlayer->unitLimit)
		return;

	//Unit Ratio to maintain
	float unitRatio[] = { 0.1f, 0.5f, 0.4f };

	float greatestDifference = -1.0f;
	int nextUnitToBuild = -1;

	//Calculate unit difference
	for (int i = 0; i < 3; i++)
	{
		int aim = unitRatio[i] * totNumUnits;
		float diff = (aim - numUnits[i]) / (float)totNumUnits;

		if (diff > greatestDifference)
		{
			greatestDifference = diff;
			nextUnitToBuild = i;
		}
	}

	//Get Proper action
	AI_ACTION* action = new TRAIN_UNIT(nextUnitToBuild);
	AI_ACTION* preAction = NULL;

	do
	{
		preAction = action->RequiresAction(m_pPlayer);
		if (preAction != NULL)
		{
			delete action;
			action = preAction;
		}
	} while (preAction != NULL);

	//Perform action
	if (action != NULL)
	{
		action->PerformAction(m_pPlayer);
		delete action;
	}
}

void MASTERAI::StrategicManager()
{
	int numScoutGroups = 0;
	int numAttackGroups = 0;

	for (int i = 0; i < m_groups.size(); i++)
	{
		if (m_groups[i]->m_task == TASK_SCOUT)
			numScoutGroups++;
		else if (m_groups[i]->m_task == TASK_ATTACK_LOCATION)
			numAttackGroups++;
	}

	//Create Scout Group
	if (m_unitPool.m_members.size() > 4 && numScoutGroups < 2)
	{
		AREA* area = m_pStrategyMap->GetScoutArea(m_pPlayer->m_teamStartLocation);

		if (area != NULL)
		{
			std::vector<int> unit;
			unit.push_back(WARRIOR);
			GROUPAI* newGroup = m_unitPool.SplitGroup(unit);

			if (newGroup != NULL)
			{
				newGroup->SetTask(TASK_SCOUT, &area->m_mapArea);
				m_groups.push_back(newGroup);
			}
		}
	}

	//Small attack
	if (m_unitPool.m_members.size() > 8 && numAttackGroups == 0)
	{
		AREA* attackArea = m_pStrategyMap->GetAttackArea(m_pPlayer->m_teamStartLocation);
		int numWarriors = 3 + rand() % 4;

		std::vector<int> units;
		for (int i = 0; i < numWarriors; i++)
			units.push_back(WARRIOR);

		//Create attack group
		GROUPAI* newGroup = m_unitPool.SplitGroup(units);

		if (attackArea->Connected(m_pTerrain, newGroup->GetCenter()))
		{
			newGroup->SetTask(TASK_ATTACK_LOCATION, &attackArea->m_mapArea);
			m_groups.push_back(newGroup);
		}
		else
		{
			newGroup->DisbandGroup();
			delete newGroup;
		}
	}
}

void MASTERAI::EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies)
{
	if (m_visibleEnemies.size() >= 30)return;

	for (int i = 0; i < manyEnemies.size(); i++)
		m_visibleEnemies.insert(manyEnemies[i]);
}