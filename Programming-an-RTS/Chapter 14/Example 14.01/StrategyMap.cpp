#include "StrategyMap.h"

//AREA

AREA::AREA(Rect area) {
	_mapArea = area;
	_status = STATUS_UNKNOWN;
	_enemyPresence = 0;
	_friendlyPresence = 0;
	_value = 0;
}

void AREA::Update(TERRAIN* pterrain, int teamNo) {
	_enemyPresence = _friendlyPresence = _value = 0;
	bool visible = false;

	if (!pterrain)
		return;

	//Calculate presence in this area
	for (int y = _mapArea.top; y < _mapArea.bottom; y++) {
		for (int x = _mapArea.left; x < _mapArea.right; x++) {
			MAPTILE* ptile = pterrain->GetTile(x, y);
			if (ptile && ptile->_pMapObject) {
				if (ptile->_pMapObject->_team == teamNo) {
					_friendlyPresence += ptile->_pMapObject->_hp * ptile->_pMapObject->_damage;
					visible = true;
				}
				else
					_enemyPresence += ptile->_pMapObject->_hp * ptile->_pMapObject->_damage;
			}
		}
	}

	//Set area status 
	if (visible) {
		if (_enemyPresence > 0 && _friendlyPresence > 0)
			_status = STATUS_CONFLICT;
		else if (_friendlyPresence > 0)
			_status = STATUS_FRIENDLY;
		else if (_enemyPresence > 0)
			_status = STATUS_ENEMY;
	}
}

bool AREA::Connected(TERRAIN* pterrain, INTPOINT from) {
	if (!pterrain)
		return false;

	MAPTILE* ptile = pterrain->GetTile(from);

	if (!ptile)
		return false;

	for (int y = _mapArea.top; y < _mapArea.bottom; y++) {
		for (int x = _mapArea.left; x < _mapArea.right; x++) {
			MAPTILE* pt = pterrain->GetTile(x, y);
			if (pt && pt->_set == ptile->_set) {
				return true;
			}
		}
	}
	return false;
}

INTPOINT AREA::GetCenter() {
	return INTPOINT(_mapArea.left + _mapArea.Width() * 0.5f, _mapArea.top + _mapArea.Height() * 0.5f);
}


//INFLUENCE MAP? (STRATEGY MAP?)
STRATEGY_MAP::STRATEGY_MAP(TERRAIN* pterrain, PLAYER* pplayer, INTPOINT numAreas) {
	_pTerrain = pterrain;
	_pPlayer = pplayer;

	if (!pterrain || !pplayer)
		return;

	//Create influence map
	_pInfluenceMap = new int[_size.x * _size.y];
	memset(_pInfluenceMap, 0, sizeof(int) * _size.x * _size.y);

	//Create sub areas
	for (int y = 0; y < numAreas.y; y++) {
		for (int x = 0; x < numAreas.x; x++) {
			Rect r = { (int)(x * (_size.x - 1) / (float)numAreas.x),
			(int)(y * (_size.y - 1) / (float)numAreas.y),
			(int)((x + 1) * (_size.x - 1) / (float)numAreas.x),
			(int)((y + 1) * (_size.y - 1) / (float)numAreas.y) };
			_areas.push_back(AREA(r));
		}
	}
}

STRATEGY_MAP::~STRATEGY_MAP() {
	if (_pInfluenceMap)
		delete[] _pInfluenceMap;
	_pInfluenceMap = nullptr;

}

void STRATEGY_MAP::Update(std::set<MAPOBJECT*>& enemies) {
	if (!_pTerrain || !_pPlayer)
		return;

	//Reset influence map
	memset(_pInfluenceMap, 0, sizeof(int) * _size.x * _size.y);

	//Add positive influence from friendly units
	for (int m = 0; m < _pPlayer->_mapObjects.size(); m++) {
		if (_pPlayer->_mapObjects[m] && !_pPlayer->_mapObjects[m]->_dead) {
			MAPOBJECT* pmo = _pPlayer->_mapObjects[m];
			Rect r = pmo->GetMapRect(pmo->_sightRadius);

			if (pmo->_pTerrain && pmo->_pTerrain->Within(pmo->_mappos)) {
				for (int y = r.top; y <= r.bottom; y++) {
					for (int x = r.left; x <= r.right; x++) {
						INTPOINT p(x, y);

						if (_pTerrain->Within(p)) {
							float multiplier = (pmo->_sightRadius - pmo->_mappos.Distance(p)) / pmo->_sightRadius;
							if (multiplier < 0.f)
								multiplier = 0.f;
							_pInfluenceMap[y * _size.x + x] += pmo->_hp * pmo->_damage * multiplier;
						}
					}
				}
			}
		}
	}

	//Subtract negative influence fron enemy units
	
	for (auto enemy : enemies) {
		if (enemy && !enemy->_dead) {
			Rect r = enemy->GetMapRect(enemy->_sightRadius);
			if (enemy->_pTerrain && enemy->_pTerrain->Within(enemy->_mappos)) {
				for (int y = r.top; y <= r.bottom; y++) {
					for (int x = r.left; x <= r.top; x++) {
						INTPOINT p(x, y);

						if (_pTerrain->Within(p)) {
							float multiplier = (enemy->_sightRadius - enemy->_mappos.Distance(p)) / enemy->_sightRadius;
							if (multiplier < 0.f)
								multiplier = 0.f;
							_pInfluenceMap[y * _size.x + x] -= enemy->_hp * enemy->_damage * multiplier;
						}
					}
				}
			}
		}
	}

	//Update map area
	for (int i = 0; i < _areas.size(); i++) {
		_areas[i].Update(_pTerrain, _pPlayer->_teamNo);
	}
}

AREA* STRATEGY_MAP::GetScoutArea(INTPOINT from) {
	int index = -1;
	float distance = 100000.f;

	//Find closes unexplored area
	for (int i = 0; i < _areas.size(); i++) {
		INTPOINT areaCentre = _areas[i].GetCenter();
		if (_areas[i]._status == STATUS_UNKNOWN && _areas[i].Connected(_pTerrain, from)) {
			float d = from.Distance(areaCentre) + _pPlayer->_teamStartLocation.Distance(areaCentre) * 0.6f;
			if (d < distance) {
				index = i;
				distance = d;
			}
		}
	}

	if (index >= 0)
		return &_areas[index];
	return GetRandomArea();
}

AREA* STRATEGY_MAP::GetAttackArea(INTPOINT from) {
	int index = -1;
	float distance = 100000.f;
	//Find closest enemy occumpied area
	for (int i = 0; i < _areas.size(); i++) {
		INTPOINT areaCentre = _areas[i].GetCenter();

		if (_areas[i]._status == STATUS_ENEMY || _areas[i]._status == STATUS_CONFLICT) {
			float d = from.Distance(areaCentre) + _pPlayer->_teamStartLocation.Distance(areaCentre) * 0.6f;

			if (d < distance) {
				index = i;
				distance = d;
			}
		}
	}
	if (index >= 0) {
		return &_areas[index];
	}
	return GetRandomArea();
}

AREA* STRATEGY_MAP::GetRandomArea() {
	return &_areas[rand() % _areas.size()];
}