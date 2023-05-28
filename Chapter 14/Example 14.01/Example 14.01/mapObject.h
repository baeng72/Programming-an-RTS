#ifndef _MAP_OBJECT_
#define _MAP_OBJECT_

#include "terrain.h"
#include "intpoint.h"
#include "mouse.h"
#include "effect.h"

//Global Functions
void LoadMapObjectResources(IDirect3DDevice9* m_pDevice);
void UnloadMapObjectResources();
INTPOINT GetScreenPos(D3DXVECTOR3 pos, IDirect3DDevice9* m_pDevice);

class GROUPAI;

class MAPOBJECT
{
public:
	//Functions
	MAPOBJECT();							//Set all variables to 0
	RECT GetMapRect(int border);			//Get map rectangle + border
	INTPOINT GetAttackPos(INTPOINT from);	//Get position to attack this mapobject from
	void PaintSelected(float time);			//Paint m_selected
	void RenderSightMesh();					//Render sightTexture & mesh for this mapobject
	std::vector<MAPOBJECT*> GetTargetsWithinRange(int theRange);	//Returns any enemy targets within m_range
	MAPOBJECT* BestTargetToAttack(std::vector<MAPOBJECT*>& enemies);

	//Virtual Functions
	virtual void Render() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual BBOX GetBoundingBox() = 0;		//Bounding box in world space
	virtual D3DXMATRIX GetWorldMatrix() = 0;
	virtual bool isDead() = 0;
	virtual void Damage(int dmg, MAPOBJECT* attacker) = 0;

	//Variables
	TERRAIN* m_pTerrain;			//Used for unit pathfinding, building placement etc
	int m_hp, m_hpMax;				//Health and max health
	int m_range;					//Attack m_range
	int m_damage;
	INTPOINT m_mappos, m_mapsize;	//Location and m_mapsize
	float m_sightRadius;
	int m_team, m_type;
	bool m_selected, m_dead;
	std::string m_name;
	MAPOBJECT* m_pTarget;			//Used for targeting both units and buildings
	D3DXVECTOR3 m_position;		//Actual world position
	IDirect3DDevice9* m_pDevice;

	bool m_isBuilding;			//Used when casting pointers...
	bool m_visible;				//Used to cull with FogOfWar
	GROUPAI* m_pGroup;
};

#endif