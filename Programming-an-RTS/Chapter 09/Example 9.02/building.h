#pragma once

#include "mapObject.h"
#include "Mesh.h"
#include "terrain.h"

void LoadBuildingResources(Renderer::RenderDevice* pdevice);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* pterrain);

class BUILDING : public MAPOBJECT {
	BBOX _BBox;
	MESHINSTANCE _meshInstance;
	bool _affectTerrain;
	
public:
	BUILDING(int type, INTPOINT mp, TERRAIN* terrain, bool affectTerrain);
	~BUILDING();
	
	void Render(Renderer::Shader*pshader);
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	mat4 GetWorldMatrix();
};