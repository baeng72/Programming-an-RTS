#ifndef _RTS_AI_ACTION_
#define _RTS_AI_ACTION_

#include <vector>
#include "intpoint.h"
#include "building.h"
#include "unit.h"

class PLAYER;

class AI_ACTION
{
public:
	virtual AI_ACTION* RequiresAction(PLAYER* player) = 0;
	virtual void PerformAction(PLAYER* player) = 0;
};

class TRAIN_UNIT : public AI_ACTION
{
public:
	TRAIN_UNIT(int unitType);
	AI_ACTION* RequiresAction(PLAYER* player);
	void PerformAction(PLAYER* player);
private:
	int m_unitToTrain;
};

class CONSTRUCT_BUILDING : public AI_ACTION
{
public:
	CONSTRUCT_BUILDING(int buildingType);
	AI_ACTION* RequiresAction(PLAYER* player);
	void PerformAction(PLAYER* player);
private:
	int m_buildingToMake;
};

#endif	