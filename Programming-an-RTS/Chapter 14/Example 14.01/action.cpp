#include "action.h"
#include "player.h"
#include "strategyMap.h"

//TRAIN UNIT
TRAIN_UNIT::TRAIN_UNIT(int unitType) {
	_unitToTrain = unitType;
}

AI_ACTION* TRAIN_UNIT::RequiresAction(PLAYER* player) {
	//Check that the needed building exists
	if (_unitToTrain == WORKER) {
		if (!player->HasMapObject(TOWNHALL, true)) {
			if (player->HasMapObject(WORKER, false))
				return new CONSTRUCT_BUILDING(TOWNHALL);
			return nullptr;
		}
	}
	else if (_unitToTrain == SOLDIER) {
		if (!player->HasMapObject(BARRACKS, true))
			return new CONSTRUCT_BUILDING(BARRACKS);
	}
	else if (_unitToTrain == MAGICIAN) {
		if (!player->HasMapObject(TOWER, true))
			return new CONSTRUCT_BUILDING(TOWER);
	}
	return nullptr;
}

void TRAIN_UNIT::PerformAction(PLAYER* player) {
	//Check that the player has cash to train the unit
	if (player->money < GetCost(_unitToTrain, false))
		return;

	//Get an available building
	BUILDING* pbuilding = player->GetAvailableBuilding(_unitToTrain);
	if (!pbuilding)
		return;

	//start training the unit
	pbuilding->TrainUnit(_unitToTrain);
}


//CONSTRUCT BUILDING
CONSTRUCT_BUILDING::CONSTRUCT_BUILDING(int buildingType) {
	_buildingToMake = buildingType;
}

AI_ACTION* CONSTRUCT_BUILDING::RequiresAction(PLAYER* player) {
	//check that we have a worker
	if (!player->HasMapObject(WORKER, false))
		return new TRAIN_UNIT(WORKER);

	//check that we have all
	if (_buildingToMake == BARRACKS || _buildingToMake == TOWER) {
		if (!player->HasMapObject(TOWNHALL, true))
			return new CONSTRUCT_BUILDING(TOWNHALL);
	}

	if (_buildingToMake == TOWER) {
		if (!player->HasMapObject(BARRACKS, true))
			return new CONSTRUCT_BUILDING(BARRACKS);
	}
	return nullptr;

}

void CONSTRUCT_BUILDING::PerformAction(PLAYER* player) {
	//Check that the player has cash to train the unit
	if (player->money < GetCost(_buildingToMake, true))
		return;

	//Get building position
	INTPOINT buildPos = player->FindClosestBuildingLocation(_buildingToMake, player->_teamStartLocation);

	//Get available worker
	UNIT* pworker = player->GetAvailableUnit(WORKER);
	if (!pworker)
		return;

	//Build building
	pworker->ConstructBuilding(_buildingToMake, buildPos);
}