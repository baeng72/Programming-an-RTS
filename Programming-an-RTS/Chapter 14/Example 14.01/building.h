#pragma once

#include "mapObject.h"
#include "Mesh.h"
#include "terrain.h"

void LoadBuildingResources(Renderer::RenderDevice* pdevice);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* pterrain);

#define TOWNHALL 0
#define BARRACKS 1
#define TOWER 2

class BUILDING : public MAPOBJECT {
	friend class MAPOBJECT;
	friend class PLAYER;
	friend class TRAIN_UNIT;
	//building fires
	vec3 _firePos[3];
	vec3 _fireScale[3];
	std::vector<EFFECT_FIRE*> _fires;

	float _deathCountDown;
	float _buildProgress;
	float _buildSpeed;
	BBOX _BBox;
	MESHINSTANCE _meshInstance;
	bool _affectTerrain;
	PLAYER* _pPlayer;

	//unit training variables
	bool _training;
	float _trainingTime;
	float _maxTrainingTime;
	int _trainingUnit;
	EFFECT_TRAINING* _pTrainingEffect;
public:
	BUILDING(Renderer::RenderDevice*pdevice,int type, int team,bool finished, INTPOINT mp, TERRAIN* terrain,PLAYER*player, bool affectTerrain);
	virtual ~BUILDING();

	void Render(Renderer::Shader* pshader)override;
	void RenderFires(mat4&matProj,mat4&matView);
	void Update(float deltaTime)override;
	BBOX GetBoundingBox()override;
	mat4 GetWorldMatrix()override;
	bool isDead();
	void Damage(int dmg, MAPOBJECT* pattacker);
	void TrainUnit(int unit);
	Renderer::Shader* GetShader()override { return nullptr; }
};