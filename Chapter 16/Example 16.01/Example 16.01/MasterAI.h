#ifndef _RTS_MASTERAI_
#define _RTS_MASTERAI_

#include <vector>
#include <stack>
#include <set>

#include "GroupAI.h"
#include "player.h"
#include "action.h"

class STRATEGY_MAP;

class MASTERAI
{
	friend class GROUPAI;
	friend class APPLICATION;
	friend class SMALL_ATTACK;
public:
	MASTERAI(PLAYER* _player, TERRAIN* _terrain);
	~MASTERAI();

	void Update(float deltaTime);
	void HandleGroups();
	void MasterAI();

	void BuildManager();
	void StrategicManager();

	void EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies);

private:
	PLAYER* m_pPlayer;
	TERRAIN* m_pTerrain;
	STRATEGY_MAP* m_pStrategyMap;

	GROUPAI m_unitPool;
	std::vector<GROUPAI*> m_groups;
	std::set<MAPOBJECT*> m_visibleEnemies;

	float m_nextGroupUpdate, m_nextUpdate;
	RECT m_base;
};

#endif