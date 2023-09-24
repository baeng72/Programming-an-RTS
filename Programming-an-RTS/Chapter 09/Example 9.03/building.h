#pragma once

#include "mapObject.h"
#include "Mesh.h"
#include "terrain.h"

void LoadBuildingResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* terrain);

class BUILDING : public MAPOBJECT {
	BBOX _BBox;
	MESHINSTANCE _meshInstance;
	bool _affectTerrain;
public:
	BUILDING(int type, int team, INTPOINT mp, TERRAIN* terrain, bool affectTerrain, Renderer::RenderDevice* pdevice);
	~BUILDING();

	void Render(mat4& matVP, Renderer::DirectionalLight& light);
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	mat4 GetWorldMatrix();

};