#ifndef _RTS_PLAYER_
#define _RTS_PLAYER_

#include <vector>
#include "shader.h"
#include "camera.h"
#include "unit.h"
#include "building.h"
#include "terrain.h"
#include "masterAI.h"

void LoadPlayerResources(IDirect3DDevice9* m_pDevice);
void UnloadPlayerResources();
float GetCost(int type, bool m_isBuilding);

#define HUMAN 0
#define COMPUTER 1
#define NETWORK 2

class PLAYER
{
	friend class APPLICATION;
	friend class MASTERAI;
	friend class BUILDING;
	friend class UNIT;
	friend class CONSTRUCT_BUILDING;
	friend class SMALL_ATTACK;
	friend class STRATEGY_MAP;
public:
	PLAYER(int _teamNo, int _controller, D3DXVECTOR4 _teamCol, INTPOINT startPos, TERRAIN* _terrain, IDirect3DDevice9* _Device);
	~PLAYER();

	MAPOBJECT* AddMapObject(int type, INTPOINT mp, bool m_isBuilding, bool finished);
	void RemoveMapObject(MAPOBJECT* mapObject);

	void RenderMapObjects(CAMERA& camera);
	void PaintSelectedMapObjects(CAMERA& camera);
	void UpdateMapObjects(float deltaTime);
	INTPOINT FindClosestBuildingLocation(int buildType, INTPOINT mp);
	void Select(MOUSE& mouse);
	void UnitOrders(MOUSE& mouse, std::vector<PLAYER*>& players, CAMERA& camera);
	INTPOINT GetCenter();
	void IsMapObjectsVisible();
	void Menu(MOUSE& mouse);

	void PlaceBuilding(MOUSE& mouse, CAMERA& camera);
	bool HasMapObject(int type, bool m_isBuilding);
	RECT GetBaseArea();

	BUILDING* GetAvailableBuilding(int type);
	UNIT* GetAvailableUnit(int type);

	//Menu variables
	float money;
	int unitLimit;

private:
	IDirect3DDevice9* m_pDevice;
	std::vector<MAPOBJECT*> m_mapObjects;
	D3DXVECTOR4 m_teamColor;
	TERRAIN* m_pTerrain;
	int m_teamNo, m_buildingToPlace;
	bool m_areaSelect, m_placeBuilding;
	INTPOINT m_startSel, m_teamStartLocation;

	UNIT* m_pSelectedUnit;

	float m_time;
	float m_nextUnitUpdate;
	int m_unitUpdateIndex;
	int m_controller;
	int m_numKills;
	MASTERAI* m_pAi;
};

#endif
