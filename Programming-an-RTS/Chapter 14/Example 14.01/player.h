#pragma once

#include "camera.h"
#include "unit.h"
#include "building.h"
#include "terrain.h"
#include "MasterAI.h"

void LoadPlayerResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadPlayerResources();
float GetCost(int type, bool isBuilding);

#define HUMAN 0
#define COMPUTER 1
#define NETWORK 2

class PLAYER {
	friend class APPLICATION;
	friend class TERRAIN;
	friend class MASTERAI;
	friend class BUILDING;
	friend class UNIT;
	friend class CONSTRUCT_BUILDING;
	friend class SMALL_ATTACK;
	friend class STRATEGY_MAP;
	Renderer::RenderDevice* _pdevice;
	std::vector<MAPOBJECT*>_mapObjects;
	vec4 _teamColor;
	TERRAIN* _pTerrain;
	int _teamNo;
	int _buildingToPlace;
	bool _areaSelect;
	bool _placeBuilding;
	INTPOINT _startSel;
	INTPOINT _teamStartLocation;

	UINT* _pSelectedUnit;

	float _time;
	float _nextUnitUpdate;
	int _unitUpdateIndex{ -1 };
	int _controller;
	int _numKills;
	MASTERAI* _pAi;
	
public:
	PLAYER(int teamNo, int controller,vec4 teamCol, INTPOINT startPos, TERRAIN* terrain,Renderer::RenderDevice*pdevice);
	~PLAYER();

	MAPOBJECT* AddMapObject(int type, INTPOINT mp, bool isBuilding, bool finished);
	void RemoveMapObject(MAPOBJECT* pmapobject);

	//void AddMapObject(int type, INTPOINT mp, bool isBuilding);
	void RenderMapObjects(CAMERA& camera,Renderer::DirectionalLight&light);
	void PaintSelectedMapObjects(CAMERA& camera);
	void UpdateMapObjects(float deltaTime);
	INTPOINT FindClosestBuildingLocation(int buildType, INTPOINT mp);
	void Select(mat4&matProj,mat4&matView,MOUSE& mouse);
	void UnitOrders(MOUSE& mouse,std::vector<PLAYER*>&players, CAMERA&camera);
	INTPOINT GetCenter();
	void IsMapObjectsVisible();
	void Menu(MOUSE& mouse);

	void PlaceBuilding(MOUSE& mouse, CAMERA& camera);
	bool HasMapObject(int type, bool isBuilding);
	Rect GetBaseArea();

	BUILDING* GetAvailableBuilding(int type);
	UNIT* GetAvailableUnit(int type);

	float money;
	int unitLimit;
};