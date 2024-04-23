#pragma once

#include "skinnedMesh.h"
#include "mapObject.h"

void LoadUnitResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadUnitResources();

#define STATE_IDLE 0
#define STATE_MOVING 1
#define STATE_DEAD 3
#define STATE_SEARCH 4
#define STATE_ATTACK 5
#define STATE_RETREAT 6
#define STATE_GOTO_BUILD 7
#define STATE_BUILD 8

#define WORKER 0
#define SOLDIER 1
#define MAGICIAN 2
#define WARRIOR 1 + rand()%2	//soldier or magician

extern std::vector<EFFECT*> effects;


class UNIT : public MAPOBJECT {
	friend class APPLICATION;
	friend class PLAYER;
	friend class BUILDING;
	friend class MAPOBJECT;
	friend class GROUPAI;
	friend class CONSTRUCT_BUILDING;
	//Animation variables
	float _time;		//this units animation time
	float _speed;		//movement & animation speed
	float _pauseTime;	//time to pause
	
	float _mana;
	float _attackTime;
	int	  _animation;	//current animation, Run, Still, attack, etc
	vec3  _rotation;	//used to build world matrix
	vec3  _scale;		//

	int _state;
	vec3 _staffPos;
	int _buildingToPlace;
	INTPOINT _buildingPosition;
	PLAYER* _pPlayer;
	Mesh::AnimationController* _pcontroller;
	//Movement variables
	INTPOINT _finalGoal;
	std::vector<INTPOINT> _path;	//the active path
	vec3 _lastWP;				//last waypoint
	vec3 _nextWP;				//next waypoint
	int _activeWP;
	bool _moving;
	float _movePrc;				//0.0-1.0, used to interpolate between lastWP and nextWP
public:
	UNIT(Renderer::RenderDevice*pdevice,int type, int team, INTPOINT mp, TERRAIN* terrain,PLAYER * pPlayer);
	virtual ~UNIT();
	//Abstract functions declared in MAPOBJECT
	void Render(Renderer::Shader* pshader)override;
	void Update(float deltaTime)override;
	BBOX GetBoundingBox()override;
	mat4 GetWorldMatrix()override;

	//Specific UNIT functions
	void Goto(INTPOINT mp, bool considerUnits, bool finalGoal,int newState);	//order unit to mp
	void MoveUnit(INTPOINT to);
	vec3 GetDirection(INTPOINT p1, INTPOINT p2);
	void SetAnimation(const char* name);
	void SetAnimation(int index);
	bool CheckCollision(INTPOINT mp);
	void Pause(float time);

	bool UnitAI(bool newMapTile);
	void Attack(MAPOBJECT* ptarget);
	void ConstructBuilding(int buildToPlace, INTPOINT pos);
	bool isDead()override;
	void Damage(int dmg, MAPOBJECT* pattacker)override;
	void Heal();

	virtual Renderer::Shader* GetShader()override;
	
};