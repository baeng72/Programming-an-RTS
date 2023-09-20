#pragma once

#include "mapObject.h"
#include "Mesh.h"
#include "terrain.h"

void LoadBuildingResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* pterrain);

class BUILDING : public MAPOBJECT {
	BBOX _BBox;
	MESHINSTANCE _meshInstance;
	bool _affectTerrain;
	vec4 _color;
	vec4 _teamColor;
public:
	BUILDING(int type, INTPOINT mp, TERRAIN* terrain, bool affectTerrain);
	~BUILDING();
	void SetColor(vec4& col) { _color = col; }
	void SetTeamColor(vec4& col) { _teamColor = col; }
	void Render(mat4& matVP, Renderer::DirectionalLight& light);
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	mat4 GetWorldMatrix();
};