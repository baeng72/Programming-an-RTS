#include "action.h"
#include "player.h"
#include "strategyMap.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//								TRAIN UNIT													//
//////////////////////////////////////////////////////////////////////////////////////////////

TRAIN_UNIT::TRAIN_UNIT(int unitType)
{
	m_unitToTrain = unitType;
}

AI_ACTION* TRAIN_UNIT::RequiresAction(PLAYER* player)
{
	//Check that the needed building exist
	if (m_unitToTrain == WORKER)
	{
		if (!player->HasMapObject(TOWNHALL, true))
		{
			if (player->HasMapObject(WORKER, false))
				return new CONSTRUCT_BUILDING(TOWNHALL);
			else return NULL;
		}
	}
	else if (m_unitToTrain == SOLDIER)
	{
		if (!player->HasMapObject(BARRACKS, true))
			return new CONSTRUCT_BUILDING(BARRACKS);
	}
	else if (m_unitToTrain == MAGICIAN)
	{
		if (!player->HasMapObject(TOWER, true))
			return new CONSTRUCT_BUILDING(TOWER);
	}

	return NULL;
}

void TRAIN_UNIT::PerformAction(PLAYER* player)
{
	//Check that the player has cash to train the unit
	if (player->money < GetCost(m_unitToTrain, false))return;

	//Get an available building
	BUILDING* building = player->GetAvailableBuilding(m_unitToTrain);
	if (building == NULL)return;

	//Start training the unit
	building->TrainUnit(m_unitToTrain);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								CONSTRUCT BUILDING											//
//////////////////////////////////////////////////////////////////////////////////////////////

CONSTRUCT_BUILDING::CONSTRUCT_BUILDING(int buildingType)
{
	m_buildingToMake = buildingType;
}

AI_ACTION* CONSTRUCT_BUILDING::RequiresAction(PLAYER* player)
{
	//Check that we have a worker
	if (!player->HasMapObject(WORKER, false))
		return new TRAIN_UNIT(WORKER);

	//Check that we have all prerequisite buildings
	if (m_buildingToMake == BARRACKS || m_buildingToMake == TOWER)
		if (!player->HasMapObject(TOWNHALL, true))
			return new CONSTRUCT_BUILDING(TOWNHALL);

	if (m_buildingToMake == TOWER)
		if (!player->HasMapObject(BARRACKS, true))
			return new CONSTRUCT_BUILDING(BARRACKS);

	return NULL;
}

void CONSTRUCT_BUILDING::PerformAction(PLAYER* player)
{
	//Check that the player has cash to train the unit
	if (player->money < GetCost(m_buildingToMake, true))return;

	//Get Building position
	INTPOINT buildPos = player->FindClosestBuildingLocation(m_buildingToMake, player->m_teamStartLocation);

	//Get Available worker
	UNIT* worker = player->GetAvailableUnit(WORKER);
	if (worker == NULL)return;

	//Build building
	worker->ConstructBuilding(m_buildingToMake, buildPos);
}