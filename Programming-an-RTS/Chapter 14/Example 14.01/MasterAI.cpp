#include "MasterAI.h"
#include "StrategyMap.h"

//MASTERAI

MASTERAI::MASTERAI(PLAYER* pplayer, TERRAIN* ppterrain) :_unitPool(this) {
	_pPlayer = pplayer;
	_pTerrain = ppterrain;
	_nextGroupUpdate = 1.f;
	_nextUpdate = 5.f + _pPlayer->_teamNo;

	INTPOINT center = _pPlayer->GetCenter();
	Rect r = { center.x - 10, center.y - 10, center.x + 10, center.y + 10 };
	_base = r;

	//create influence map
	_pStrategyMap = new STRATEGY_MAP(_pTerrain, _pPlayer, INTPOINT(10, 10));
}

MASTERAI::~MASTERAI() {
	//Delete all groups 
	for (int i = 0; i < _groups.size(); i++) {
		if (_groups[i]) {
			delete _groups[i];
		}
	}
	_groups.clear();

	//release strategy map
	if (_pStrategyMap)
		delete _pStrategyMap;
}

void MASTERAI::Update(float delta) {
	if (!_pPlayer || !_pTerrain)
		return;

	//Group AI update
	_nextGroupUpdate -= delta;
	if (_nextGroupUpdate < 0.f) {
		_nextGroupUpdate = 3.f;
		HandleGroups();
	}

	//Master AI update
	_nextUpdate -= delta;
	if (_nextUpdate < 0.f) {
		_nextUpdate = 10.f;
		MasterAI();
	}


}

void MASTERAI::HandleGroups() {
	//Add unassigned units/buildings to the unit pool
	for (int m = 0; m < _pPlayer->_mapObjects.size(); m++) {
		if (_pPlayer->_mapObjects[m] && !_pPlayer->_mapObjects[m]->_dead) {
			if (!_pPlayer->_mapObjects[m]->_pGroup) {
				_unitPool.AddMember(_pPlayer->_mapObjects[m]);
			}
		}
	}

	//Update unit pool
	_base = _pPlayer->GetBaseArea();
	_unitPool.SetTask(TASK_DEFEND_LOCATION, &_base);
	_unitPool.GroupAI();

	//Update other groups
	std::vector<GROUPAI*> toDelete;
	for (auto group : _groups) {
		if (group && group->isDead())
			toDelete.push_back(group);
		else
			group->GroupAI();
	}
	for (auto del : toDelete) {
		_groups.erase(std::find(_groups.begin(), _groups.end(), del));
		delete del;
		
	}
}

void MASTERAI::MasterAI() {
	//Update influence map
	if (_pStrategyMap)
		_pStrategyMap->Update(_visibleEnemies);

	//Build buildings and train units
	BuildManager();

	//Command Units 
	StrategicManager();
}

void MASTERAI::BuildManager() {
	//Count number of buildings and units
	int totNumUnits = 0;
	int numUnits[3] = { 0,0,0 };
	int numBuildings[3] = { 0,0,0 };
	for (int i = 0; i < _pPlayer->_mapObjects.size(); i++) {
		if (_pPlayer->_mapObjects[i]) {
			if(_pPlayer->_mapObjects[i]->_isBuilding) {
				numBuildings[_pPlayer->_mapObjects[i]->_type]++;
			}
			else {
				numUnits[_pPlayer->_mapObjects[i]->_type]++;
				totNumUnits++;
			}
		}
	}

	//Can't train more units that the unit limit
	if (totNumUnits >= _pPlayer->unitLimit)
		return;

	//Unit ration to maintain
	float unitRation[] = { 0.1f,0.5,0.4f };

	float greatestDifference = -1.f;
	int nextUnitToBuild = -1;

	//Calculate unit difference
	for (int i = 0; i < 3; i++) {
		int aim = unitRation[i] * totNumUnits;
		float diff = (aim - numUnits[i]) / (float)totNumUnits;

		if (diff > greatestDifference) {
			greatestDifference = diff;
			nextUnitToBuild = i;
		}
	}

	//Get Proper action
	AI_ACTION* paction = new TRAIN_UNIT(nextUnitToBuild);
	AI_ACTION* ppreAction = nullptr;

	do {
		ppreAction = paction->RequiresAction(_pPlayer);
		if (ppreAction) {
			delete paction;
			paction = ppreAction;
		}
	} while (ppreAction);

	if (paction) {
		paction->PerformAction(_pPlayer);
		delete paction;
	}
}

void MASTERAI::StrategicManager() {
	int numScoutGroups = 0;
	int numAttackGroups = 0;

	for (auto group : _groups) {
		if (group->_task == TASK_SCOUT)
			numScoutGroups++;
		else if (group->_task == TASK_ATTACK_LOCATION)
			numAttackGroups++;
	}

	//Create scout group
	if (_unitPool._members.size() > 4 && numScoutGroups < 2) {
		AREA* parea = _pStrategyMap->GetScoutArea(_pPlayer->_teamStartLocation);

		if (parea) {
			std::vector<int> unit;
			unit.push_back(WARRIOR);
			GROUPAI* pgroup = _unitPool.SplitGroup(unit);

			if (pgroup) {
				pgroup->SetTask(TASK_SCOUT, &parea->_mapArea);
				_groups.push_back(pgroup);
			}
		}
	}

	//Small attack
	if (_unitPool._members.size() > 8 && numAttackGroups == 0) {
		AREA* pattackArea = _pStrategyMap->GetAttackArea(_pPlayer->_teamStartLocation);
		int numWarriors = 3 + rand() % 4;

		std::vector<int> units(numWarriors, WARRIOR);
		//for (int i = 0; i < numWarriors; i++)
			//units.push_back(WARRIOR);

		//Create attack group
		GROUPAI* pgroup = _unitPool.SplitGroup(units);
		if (pattackArea->Connected(_pTerrain, pgroup->GetCenter())) {
			pgroup->SetTask(TASK_ATTACK_LOCATION,&pattackArea->_mapArea);
			_groups.push_back(pgroup);			
		}
		else {
			pgroup->DisbandGroup();
			delete pgroup;
		}
	}
}

void MASTERAI::EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies) {
	if (_visibleEnemies.size() > 30)
		return;

	for (int i = 0; i < manyEnemies.size(); i++) {
		_visibleEnemies.insert(manyEnemies[i]);
	}
	//_visibleEnemies.insert(_visibleEnemies.end(), manyEnemies.begin(), manyEnemies.end());
}