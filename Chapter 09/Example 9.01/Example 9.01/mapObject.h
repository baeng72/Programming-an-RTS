#ifndef _MAP_OBJECT_
#define _MAP_OBJECT_

#include "terrain.h"
#include "intpoint.h"
#include "mouse.h"

//Global Functions
void LoadMapObjectResources(IDirect3DDevice9* Device);
void UnloadMapObjectResources();
INTPOINT GetScreenPos(D3DXVECTOR3 pos, IDirect3DDevice9* Device);


class MAPOBJECT
{
public:
	//Functions
	MAPOBJECT();					//Set all variables to 0
	RECT GetMapRect(int border);	//Get map rectangle + border
	void PaintSelected();			//Paint selected

	//Virtual Functions
	virtual void Render() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual BBOX GetBoundingBox() = 0;		//Bounding box in world space
	virtual D3DXMATRIX GetWorldMatrix() = 0;

	//Variables
	TERRAIN* m_pTerrain;			//Used for unit pathfinding, building placement etc
	int m_hp, m_hpMax;				//Health and max health
	int m_range;					//Attack range
	int m_damage;
	INTPOINT m_mappos, m_mapsize;	//Location and mapsize
	float m_sightRadius;
	int m_team, m_type;
	bool m_selected, m_dead;
	std::string m_name;
	MAPOBJECT* m_pTarget;			//Used for targeting both units and buildings
	D3DXVECTOR3 m_position;			//Actual world position
	IDirect3DDevice9* m_pDevice;

	bool m_isBuilding;				//Used when casting pointers etc...
};

#endif