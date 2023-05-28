#ifndef _RTS_STRATEGYMAP_
#define _RTS_STRATEGYMAP_

#include <vector>
#include <set>
#include "terrain.h"
#include "player.h"
#include "mapobject.h"
#include "intpoint.h"

//Area status
#define STATUS_UNKNOWN 0
#define STATUS_FRIENDLY 1
#define STATUS_ENEMY 2
#define STATUS_CONFLICT 3

struct AREA
{
	AREA(RECT mArea);
	void Update(TERRAIN* terrain, int teamNo);			//Update area information
	bool Connected(TERRAIN* terrain, INTPOINT from);	//Is "from" connected with the area
	INTPOINT GetCenter();								//returns the center of the area

	RECT m_mapArea;
	int m_status;			//Owner, conflict etc...
	int m_enemyPresence;
	int m_friendlyPresence;
	int m_value;			//Resources, strategic value etc
};

class STRATEGY_MAP
{
public:
	STRATEGY_MAP(TERRAIN* _terrain, PLAYER* _player, INTPOINT numAreas);
	~STRATEGY_MAP();

	void Update(std::set<MAPOBJECT*>& enemies);

	AREA* GetScoutArea(INTPOINT from);
	AREA* GetAttackArea(INTPOINT from);
	AREA* GetRandomArea();

private:
	TERRAIN* m_pTerrain;
	PLAYER* m_pPlayer;
	INTPOINT m_size;
	int* m_pInfluenceMap;
	std::vector<AREA> m_areas;
};

#endif