#pragma once

#include "camera.h"
#include "unit.h"
#include "building.h"
#include "terrain.h"

void LoadPlayerResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadPlayerResources();

class PLAYER {
	friend class APPLICATION;
	friend class TERRAIN;
	Renderer::RenderDevice* _pdevice;
	std::vector<MAPOBJECT*>_mapObjects;
	vec4 _teamColor;
	TERRAIN* _pTerrain;
	int _teamNo;
	bool _areaSelect;
	INTPOINT _startSel;
	
public:
	PLAYER(int teamNo, vec4 teamCol, INTPOINT startPos, TERRAIN* terrain,Renderer::RenderDevice*pdevice);
	~PLAYER();

	void AddMapObject(int type, INTPOINT mp, bool isBuilding);
	void RenderMapObjects(CAMERA& camera,Renderer::DirectionalLight&light);
	void PaintSelectedMapObjects(CAMERA& camera);
	void UpdateMapObjects(float deltaTime);
	INTPOINT FindClosestBuildingLocation(int buildType, INTPOINT mp);
	void Select(mat4&matProj,mat4&matView,MOUSE& mouse);
	void UnitOrders(MOUSE& mouse);
	INTPOINT GetCenter();
	void IsMapObjectsVisible();
};