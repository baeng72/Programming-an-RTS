#pragma once

#include "terrain.h"
#include "intpoint.h"
#include "mouse.h"

void LoadMapObjectResources(Renderer::RenderDevice* pdevice);
void UnloadMapObjectResources();
INTPOINT GetScreenPos(mat4&matVP,vec4&viewPort, vec3& pos);

class MAPOBJECT {
public:
	TERRAIN* _pTerrain;
	int _hp;				//Health
	int _hpMax;				//Max Health
	int _range;				//Attack ranger
	int _damage;			//
	INTPOINT _mappos;		//Location
	INTPOINT _mapsize;		//mapsize
	float _sightRadius;
	int _team;
	int _type;
	bool _selected;
	bool _dead;
	std::string _name;
	MAPOBJECT* _pTarget;	//Used for targeting both units and buildings
	vec3 _position;			//Actual world position
	
	bool _isBuilding;
public:
	MAPOBJECT();		//set all variables to 0;
	Rect GetMapRect(int border);	//get map rectangle + border
	void PaintSelected(mat4& matVP, vec4& viewport);			//Paint selected

	//virtual function
	virtual void Render(mat4&matVP,Renderer::DirectionalLight&light) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual BBOX GetBoundingBox() = 0;
	virtual mat4 GetWorldMatrix() = 0;
};