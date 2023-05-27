#ifndef _RTS_PLAYER_
#define _RTS_PLAYER_

#include <vector>
#include "shader.h"
#include "camera.h"
#include "unit.h"
#include "building.h"
#include "terrain.h"

void LoadPlayerResources(IDirect3DDevice9* Device);
void UnloadPlayerResources();

class PLAYER
{
	friend class APPLICATION;
public:
	PLAYER(int _teamNo, D3DXVECTOR4 _teamCol, INTPOINT startPos, TERRAIN* _terrain, IDirect3DDevice9* _Device);
	~PLAYER();

	void AddMapObject(int type, INTPOINT mp, bool isBuilding);
	void RenderMapObjects(CAMERA& camera);
	void PaintSelectedMapObjects(CAMERA& camera);
	void UpdateMapObjects(float deltaTime);
	INTPOINT FindClosestBuildingLocation(int buildType, INTPOINT mp);
	void Select(MOUSE& mouse);
	void UnitOrders(MOUSE& mouse);
	INTPOINT GetCenter();
	void IsMapObjectsVisible();

private:
	IDirect3DDevice9* m_pDevice;
	std::vector<MAPOBJECT*> m_mapObjects;
	D3DXVECTOR4 m_teamColor;
	TERRAIN* m_pTerrain;
	int m_teamNo;
	bool m_areaSelect;
	INTPOINT m_startSel;
};

#endif