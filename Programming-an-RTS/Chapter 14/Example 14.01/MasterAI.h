#pragma once

#include <vector>
#include <stack>
#include <set>

#include "GroupAI.h"
#include "player.h"
#include "action.h"

class STRATEGY_MAP;

class MASTERAI {
	friend class GROUPAI;
	friend class APPLICATION;
	friend class SMALL_ATTACK;
	PLAYER *_pPlayer;
	TERRAIN* _pTerrain;
	STRATEGY_MAP* _pStrategyMap;

	GROUPAI _unitPool;

	std::vector<GROUPAI*> _groups;
	std::set<MAPOBJECT*> _visibleEnemies;

	float _nextGroupUpdate;
	float _nextUpdate;
	Rect _base;
public:
	MASTERAI(PLAYER* pplayer, TERRAIN* pterrain);
	~MASTERAI();

	void Update(float delta);
	void HandleGroups();
	void MasterAI();

	void BuildManager();
	void StrategicManager();

	void EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies);
};