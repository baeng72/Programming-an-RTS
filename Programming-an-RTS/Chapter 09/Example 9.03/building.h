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
	BUILDING(int type,int team, INTPOINT mp, TERRAIN* terrain, bool affectTerrain);
	~BUILDING();
	
	void Render(Renderer::Shader*pshader)override;
	void Update(float deltaTime)override;
	BBOX GetBoundingBox()override;
	mat4 GetWorldMatrix()override;
	Renderer::Shader* GetShader()override { return nullptr; }
};