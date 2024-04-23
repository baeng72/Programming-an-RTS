#pragma once

#include <vector>
#include "intpoint.h"
#include "building.h"
#include "unit.h"

class PLAYER;

class AI_ACTION {
public:
	virtual AI_ACTION* RequiresAction(PLAYER* player) = 0;
	virtual void PerformAction(PLAYER* player) = 0;
};

class TRAIN_UNIT : public AI_ACTION {
	int _unitToTrain;
public:
	TRAIN_UNIT(int unitType);
	AI_ACTION* RequiresAction(PLAYER* player)override;
	void PerformAction(PLAYER* player)override;
};

class CONSTRUCT_BUILDING : public AI_ACTION {
	int _buildingToMake;
public:
	CONSTRUCT_BUILDING(int buildingType);
	AI_ACTION* RequiresAction(PLAYER* player)override;
	void PerformAction(PLAYER* player)override;
};