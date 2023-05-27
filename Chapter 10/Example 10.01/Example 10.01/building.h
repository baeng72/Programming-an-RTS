#ifndef _RTS_BUILDING_
#define _RTS_BUILDING_

#include "mapObject.h"
#include "mesh.h"
#include "debug.h"
#include "terrain.h"

void LoadBuildingResources(IDirect3DDevice9* Device);
void UnloadBuildingResources();
bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* terrain);

class BUILDING : public MAPOBJECT
{
public:
	BUILDING(int _type, int _team, INTPOINT mp, TERRAIN* _terrain, bool _affectTerrain, IDirect3DDevice9* Dev);
	~BUILDING();

	void Render();
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	D3DXMATRIX GetWorldMatrix();

private:

	BBOX m_BBox;
	MESHINSTANCE m_meshInstance;
	bool m_affectTerrain;
};

#endif