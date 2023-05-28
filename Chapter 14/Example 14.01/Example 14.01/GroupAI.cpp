#include "groupAI.h"
#include "masterAI.h"
#include "strategyMap.h"

GROUPAI::GROUPAI(MASTERAI* _master)
{
	m_pMaster = _master;
	m_task = TASK_NONE;
	m_state = GROUP_STATE_IDLE;
}

GROUPAI::~GROUPAI()
{
	//Reset group pointer for all group members
	for (int i = 0; i < m_members.size(); i++)
		m_members[i]->m_pGroup = NULL;
}

void GROUPAI::AddMember(MAPOBJECT* newMember)
{
	if (Find(m_members, newMember) < 0)
	{
		newMember->m_pGroup = this;
		m_members.push_back(newMember);
	}
}

void GROUPAI::RemoveMember(MAPOBJECT* oldMember)
{
	int place = Find(m_members, oldMember);

	if (place >= 0)
	{
		oldMember->m_pGroup = NULL;

		//m_members.erase(&m_members[place]);
		std::vector<MAPOBJECT*>::iterator it = m_members.begin() + place;
		m_members.erase(it);
	}
}

void GROUPAI::DisbandGroup()
{
	while (!m_members.empty())
	{
		RemoveMember(m_members[0]);
	}

	m_members.clear();
	m_visibleEnemies.clear();
}

void GROUPAI::EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies)
{
	//This function is used by units to report spotted enemy units

	if (m_visibleEnemies.size() > 30)return;

	for (int i = 0; i < manyEnemies.size(); i++)
		if (Find(m_visibleEnemies, manyEnemies[i]) < 0)
			m_visibleEnemies.push_back(manyEnemies[i]);
}

void GROUPAI::SetTask(int newTask, RECT* area)
{
	m_task = newTask;

	if (area == NULL)
		m_task = TASK_NONE;
	else m_mapArea = *area;
}

GROUPAI* GROUPAI::SplitGroup(std::vector<int> units)
{
	//This function splits the group according to the wishlist (units)

	if (units.empty() || m_members.empty())return NULL;

	GROUPAI* newGroup = new GROUPAI(m_pMaster);

	try
	{
		bool done = false;

		while (!done)
		{
			//Transfer member
			for (int i = 0; i < m_members.size(); i++)
				if (!m_members[i]->m_isBuilding && m_members[i]->m_type == units[0])
				{
					MAPOBJECT* unit = m_members[i];
					RemoveMember(unit);
					newGroup->AddMember(unit);
					break;
				}

			units.erase(units.begin());
			done = units.empty() || m_members.empty();
		}

		if (newGroup->isDead())
		{
			delete newGroup;
			newGroup = NULL;
		}
	}
	catch (...)
	{
		debug.Print("Error in GROUPAI::SplitGroup()");
	}

	return newGroup;
}

void GROUPAI::Goto(RECT mArea)
{
	if (m_members.empty())return;

	//Move group to area

	try
	{
		for (int i = 0; i < m_members.size(); i++)
			if (m_members[i] != NULL && !m_members[i]->m_isBuilding && !m_members[i]->m_dead)
			{
				UNIT* unit = (UNIT*)m_members[i];

				if (unit->m_state != STATE_ATTACK)
				{
					INTPOINT p(rand() % (mArea.right - mArea.left) + mArea.left,
						rand() % (mArea.bottom - mArea.top) + mArea.top);

					p = unit->m_pTerrain->GetClosestFreeTile(p, unit->m_mappos);
					unit->m_pTarget = NULL;
					unit->Goto(p, false, true, STATE_MOVING);
				}
			}
	}
	catch (...) {}
}

void GROUPAI::Attack(std::vector<MAPOBJECT*>& enemies)
{
	//Attack enemies

	for (int i = 0; i < m_members.size(); i++)
		if (!m_members[i]->m_isBuilding && !m_members[i]->m_dead)
		{
			UNIT* unit = (UNIT*)m_members[i];

			if (unit->m_state == STATE_IDLE || unit->m_state == STATE_MOVING)
				unit->Attack(unit->BestTargetToAttack(enemies));
		}
}

void GROUPAI::RetreatTo(RECT ma)
{
	if (m_members.empty())return;

	for (int i = 0; i < m_members.size(); i++)
		if (!m_members[i]->m_isBuilding && !m_members[i]->m_dead)
		{
			UNIT* unit = (UNIT*)m_members[i];

			if (!unit->m_mappos.inRect(ma))
			{
				INTPOINT p(rand() % (ma.right - ma.left) + ma.left,
					rand() % (ma.bottom - ma.top) + ma.top);

				p = unit->m_pTerrain->GetClosestFreeTile(p, unit->m_mappos);
				unit->m_pTarget = NULL;
				unit->Goto(p, false, true, STATE_RETREAT);
			}
		}

	m_mapArea = ma;
}

void GROUPAI::Shuffle()
{
	//Random movement

	if (m_members.empty())return;

	for (int i = 0; i < m_members.size(); i++)
		if (!m_members[i]->m_isBuilding && !m_members[i]->m_dead && rand() % 40 == 0)
		{
			UNIT* unit = (UNIT*)m_members[i];

			INTPOINT p(rand() % (m_mapArea.right - m_mapArea.left) + m_mapArea.left,
				rand() % (m_mapArea.bottom - m_mapArea.top) + m_mapArea.top);

			p = unit->m_pTerrain->GetClosestFreeTile(p, unit->m_mappos);
			unit->m_pTarget = NULL;
			unit->Goto(p, false, true, STATE_MOVING);
		}
}

void GROUPAI::GroupAI()
{
	if (m_members.empty() || m_pMaster == NULL)return;

	int memberStates[] = { 0, 0, 0 };
	m_state = GROUP_STATE_IDLE;

	std::vector<MAPOBJECT*>::iterator i;
	for (i = m_members.begin(); i != m_members.end();)
	{
		if ((*i) == NULL || (*i)->m_dead)
		{
			//Remove dead Group m_members
			RemoveMember(*i);
		}
		else
		{
			(*i)->m_pGroup = this;

			//determine group m_state
			if (!(*i)->m_isBuilding)
			{
				UNIT* unit = (UNIT*)(*i);

				if (unit->m_state == STATE_ATTACK)
					memberStates[GROUP_STATE_BATTLE]++;
				else if (unit->m_moving)
					memberStates[GROUP_STATE_MOVING]++;
				else memberStates[GROUP_STATE_IDLE]++;
			}
			i++;
		}
	}

	//Set group state
	if (memberStates[GROUP_STATE_BATTLE] >= m_members.size() * 0.2f)
		m_state = GROUP_STATE_BATTLE;
	else if (memberStates[GROUP_STATE_MOVING] >= m_members.size() * 0.4f)
		m_state = GROUP_STATE_MOVING;
	else m_state = GROUP_STATE_IDLE;

	//Group state machine
	switch (m_state)
	{
	case GROUP_STATE_IDLE:
	{
		if (m_task == TASK_SCOUT)
		{
			AREA* area = m_pMaster->m_pStrategyMap->GetScoutArea(GetCenter());
			if (area != NULL)Goto(area->m_mapArea);
		}
		else if (m_task == TASK_ATTACK_LOCATION)
		{
			AREA* area = m_pMaster->m_pStrategyMap->GetAttackArea(GetCenter());
			if (area != NULL)Goto(area->m_mapArea);
		}
		else if (m_task == TASK_NONE || m_task == TASK_DEFEND_LOCATION)
		{
			Shuffle();
		}

		break;
	}
	case GROUP_STATE_MOVING:
	{
		Attack(m_visibleEnemies);

		break;
	}
	case GROUP_STATE_BATTLE:
	{
		Attack(m_visibleEnemies);

		if (m_task == TASK_DEFEND_LOCATION)
			RetreatTo(m_mapArea);

		break;
	}
	}

	//Report enemies to Master AI
	m_pMaster->EnemiesSpotted(m_visibleEnemies);
	m_visibleEnemies.clear();
}

bool GROUPAI::isDead()
{
	return m_members.empty();
}

INTPOINT GROUPAI::GetCenter()
{
	INTPOINT p;

	try
	{
		if (m_members.size() <= 0)return p;

		for (int i = 0; i < m_members.size(); i++)
			p += m_members[i]->m_mappos;

		p /= m_members.size();
	}
	catch (...)
	{
		debug.Print("Error in GROUPAI::GetCenter()");
	}

	return p;
}