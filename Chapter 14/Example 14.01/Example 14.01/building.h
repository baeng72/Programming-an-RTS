#ifndef _RTS_BUILDING_
#define _RTS_BUILDING_

#include "mapObject.h"
#include "mesh.h"
#include "debug.h"
#include "terrain.h"

class PLAYER;

void LoadBuildingResources(IDirect3DDevice9* m_pDevice);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* terrain);

#define TOWNHALL 0
#define BARRACKS 1
#define TOWER 2

class BUILDING : public MAPOBJECT
{
	friend class MAPOBJECT;
	friend class PLAYER;
	friend class TRAIN_UNIT;
public:
	BUILDING(int _type, int _team, bool finished, INTPOINT mp, TERRAIN* _terrain, PLAYER* _player, bool _affectTerrain, IDirect3DDevice9* Dev);
	~BUILDING();

	void Render();
	void RenderFires();

	void Update(float deltaTime);
	BBOX GetBoundingBox();
	D3DXMATRIX GetWorldMatrix();
	bool isDead();
	void Damage(int dmg, MAPOBJECT* attacker);

	void TrainUnit(int unit);

private:

	//Building m_fires
	D3DXVECTOR3 m_firePos[3], m_fireScale[3];
	std::vector<EFFECT_FIRE*> m_fires;

	float m_deathCountDown, m_buildProgress, m_buildSpeed;
	BBOX m_BBox;
	MESHINSTANCE m_meshInstance;
	bool m_affectTerrain;
	PLAYER* m_pPlayer;

	//Unit Training variables
	bool m_training;
	float m_trainingTime, m_maxTrainingTime;
	int m_trainingUnit;
	EFFECT_TRAINING* m_pTrainingEffect;
};

#endif