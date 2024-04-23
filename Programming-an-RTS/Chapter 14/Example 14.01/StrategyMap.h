#pragma once

#include <vector>
#include <set>
#include "terrain.h"
#include "player.h"
#include "mapObject.h"
#include "intpoint.h"

//Area status
#define STATUS_UNKNOWN 0
#define STATUS_FRIENDLY 1
#define STATUS_ENEMY 2
#define STATUS_CONFLICT 3

struct AREA {
	Rect _mapArea;
	int _status;//owner, conflict, etc
	int _enemyPresence;
	int _friendlyPresence;
	int _value; //Resources, strategic value etc

	AREA(Rect area);
	void Update(TERRAIN* pterrain, int teamNo); //Update area info
	bool Connected(TERRAIN* pterrain, INTPOINT from);//is "from" connected with the area
	INTPOINT GetCenter();	//returns the center of the area
};

class STRATEGY_MAP {
	TERRAIN* _pTerrain;
	PLAYER* _pPlayer;
	INTPOINT _size;
	int* _pInfluenceMap;
	std::vector<AREA> _areas;
public:
	STRATEGY_MAP(TERRAIN* pterrain, PLAYER* pplayer, INTPOINT numAreas);
	~STRATEGY_MAP();

	void Update(std::set<MAPOBJECT*>& enemies);

	AREA* GetScoutArea(INTPOINT from);
	AREA* GetAttackArea(INTPOINT from);
	AREA* GetRandomArea();
};