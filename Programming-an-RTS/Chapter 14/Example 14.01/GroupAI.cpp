#include "GroupAI.h"
#include "MasterAI.h"
#include "StrategyMap.h"

GROUPAI::GROUPAI(MASTERAI* pmaster) {
	_pMaster = pmaster;
	_task = TASK_NONE;
	_state = GROUP_STATE_IDLE;
}

GROUPAI::~GROUPAI() {
	//Reset group pointer for all group members
	for (int i = 0; i < _members.size(); i++) {
		_members[i]->_pGroup;
	}
}

void GROUPAI::AddMember(MAPOBJECT* newMember) {
	if (Find(_members, newMember) < 0) {
		newMember->_pGroup = this;
		_members.push_back(newMember);
	}
}

void GROUPAI::RemoveMember(MAPOBJECT* oldMember) {
	int place = Find(_members, oldMember);

	if (place >= 0) {
		oldMember->_pGroup = nullptr;

		std::vector<MAPOBJECT*>::iterator it = _members.begin() + place;
		_members.erase(it);
	}
}

void GROUPAI::DisbandGroup() {
	while (!_members.empty()) {
		RemoveMember(_members[0]);
	}
	_members.clear();
	_visibleEnemies.clear();
}

void GROUPAI::EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies) {
	//This function is used by units to report spotted enemy units

	if (_visibleEnemies.size() > 30)
		return;

	for (int i = 0; i < manyEnemies.size(); i++) {
		if (Find(_visibleEnemies, manyEnemies[i]) < 0) {
			_visibleEnemies.push_back(manyEnemies[i]);
		}
	}
}

void GROUPAI::SetTask(int newTask, Rect* area) {
	_task = newTask;

	if (!area) {
		_task = TASK_NONE;
	}
	else {
		_mapArea = *area;
	}
}

GROUPAI* GROUPAI::SplitGroup(std::vector<int>& units) {
	//This function splits the group according to the wishlist (units)

	if (units.empty() || _members.empty())
		return nullptr;

	GROUPAI* newGroup = new GROUPAI(_pMaster);

	bool done = false;
	while (!done) {
		//Transfer member
		for (int i = 0; i < _members.size(); i++) {
			if (!_members[i]->_isBuilding && _members[i]->_type == units[0]) {
				MAPOBJECT* punit = _members[i];
				RemoveMember(punit);
				newGroup->AddMember(punit);
				break;
			}
		}
		units.erase(units.begin());
		done = units.empty() || _members.empty();
	}

	if (newGroup->isDead()) {
		delete newGroup;
		newGroup = nullptr;
	}

	return newGroup;
}

void GROUPAI::Goto(Rect area) {
	if (_members.empty())
		return;

	//Move group to area

	for (int i = 0; i < _members.size(); i++) {
		if (_members[i] && !_members[i]->_isBuilding && !_members[i]->_dead) {
			UNIT* punit = (UNIT*)_members[i];

			if (punit->_state != STATE_ATTACK) {
				INTPOINT p(rand() % (area.right - area.left)+area.left,
					rand() % (area.bottom - area.top) + area.top);

				p = punit->_pTerrain->GetClosestFreeTile(p, punit->_mappos);
				punit->_pTerrain = nullptr;
				punit->Goto(p, false, true, STATE_MOVING);
			}
		}
	}
}

void GROUPAI::Attack(std::vector<MAPOBJECT*>& enemies) {
	//Attack enemies
	for (int i = 0; i < _members.size(); i++) {
		if (!_members[i]->_isBuilding && !_members[i]->_dead) {
			UNIT* punit = (UNIT*)_members[i];
			if (punit->_state == STATE_IDLE || punit->_state == STATE_MOVING)
				punit->Attack(punit->BestTargetToAttack(enemies));
		}
	}
}

void GROUPAI::RetreatTo(Rect ma) {
	if (_members.empty())
		return;

	for (int i = 0; i < _members.size(); i++) {
		if (!_members[i]->_isBuilding && !_members[i]->_dead) {
			UNIT* punit = (UNIT*)_members[i];

			if (!punit->_mappos.inRect(ma)) {
				INTPOINT p(rand() % (ma.right - ma.left) + ma.left, rand() % (ma.bottom - ma.top) + ma.top);
				p = punit->_pTerrain->GetClosestFreeTile(p, punit->_mappos);
				punit->_pTarget = nullptr;
				punit->Goto(p, false, true, STATE_RETREAT);
			}
		}
	}
	_mapArea = ma;
}

void GROUPAI::Shuffle() {
	//Random movement

	if (_members.empty())
		return;

	for (int i = 0; i < _members.size(); i++) {
		if (!_members[i]->_isBuilding && !_members[i]->_dead && rand() % 40 == 0) {
			UNIT* punit = (UNIT*)_members[i];

			INTPOINT p(rand() % (_mapArea.right - _mapArea.left) + _mapArea.left,
				rand() % (_mapArea.bottom - _mapArea.top) + _mapArea.top);

			p = punit->_pTerrain->GetClosestFreeTile(p, punit->_mappos);
			punit->_pTarget = nullptr;
			punit->Goto(p, false, true, STATE_MOVING);
		}
	}
}

void GROUPAI::GroupAI() {
	if (_members.empty())
		return;

	int memberStates[] = { 0,0,0 };
	_state = GROUP_STATE_IDLE;
	std::vector<MAPOBJECT*> toDelete;
	for (auto pobj : _members) {
		if(!pobj || pobj->_dead)
			toDelete.push_back(pobj);
		else {
			pobj->_pGroup = this;

			//determine group state
			if (!pobj->_isBuilding) {
				UNIT* punit = (UNIT*)pobj;
				if (punit->_state == STATE_ATTACK)
					memberStates[GROUP_STATE_BATTLE]++;
				else if (punit->_moving)
					memberStates[GROUP_STATE_MOVING]++;
				else
					memberStates[GROUP_STATE_IDLE]++;
					
			}
		}
	}
	for (auto& del : toDelete) {
		RemoveMember(del);
	}

	//Set group state
	if (memberStates[GROUP_STATE_BATTLE] >= _members.size() * 0.2f)
		_state = GROUP_STATE_BATTLE;
	else if (memberStates[GROUP_STATE_MOVING] >= _members.size() * 0.4f)
		_state = GROUP_STATE_MOVING;
	else
		_state = GROUP_STATE_IDLE;

	//Group state machin
	switch (_state) {
	case GROUP_STATE_IDLE:
	{
		if (_task == TASK_SCOUT) {
			AREA* parea = _pMaster->_pStrategyMap->GetScoutArea(GetCenter());
			if (parea)
				Goto(parea->_mapArea);
		}
		else if (_task == TASK_ATTACK_LOCATION) {
			AREA* parea = _pMaster->_pStrategyMap->GetScoutArea(GetCenter());
			if (parea)
				Goto(parea->_mapArea);
		}
		else if (_task == TASK_NONE || _task == TASK_DEFEND_LOCATION) {
			Shuffle();
		}
		break;
	}
	case GROUP_STATE_MOVING:
		Attack(_visibleEnemies);
		break;
	case GROUP_STATE_BATTLE:
		Attack(_visibleEnemies);
		if (_task == TASK_DEFEND_LOCATION)
			RetreatTo(_mapArea);
		break;
	}

	_pMaster->EnemiesSpotted(_visibleEnemies);
	_visibleEnemies.clear();
}

bool GROUPAI::isDead() {
	return _members.empty();
}

INTPOINT GROUPAI::GetCenter() {
	INTPOINT p;
	if (_members.size() <= 0)
		return p;

	for (int i = 0; i < _members.size(); i++) {
		p += _members[i]->_mappos;
	}
	p /= (int)_members.size();
	return p;

}