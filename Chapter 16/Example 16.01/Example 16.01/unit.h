#ifndef _RTS_UNIT_
#define _RTS_UNIT_

#include "skinnedmesh.h"
#include "mapObject.h"

void LoadUnitResources(IDirect3DDevice9* m_pDevice);
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
#define WARRIOR 1 + rand()%2		//Soldier or Magician

//Effect Pool
extern std::vector<EFFECT*> effects;
class PLAYER;
class UNIT : public MAPOBJECT
{
	friend class APPLICATION;
	friend class PLAYER;
	friend class BUILDING;
	friend class MAPOBJECT;
	friend class GROUPAI;
	friend class CONSTRUCT_BUILDING;
public:
	UNIT(int _type, int _team, INTPOINT mp, TERRAIN* _terrain, PLAYER* _player, IDirect3DDevice9* Dev);
	~UNIT();

	//Abstract functions declared in MAPOBJECT
	void Render();
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	D3DXMATRIX GetWorldMatrix();

	//Specific UNIT functions
	void Goto(INTPOINT mp, bool considerUnits, bool _finalGoal, int newState);	//Order unit to mp
	void MoveUnit(INTPOINT to);
	D3DXVECTOR3 GetDirection(INTPOINT p1, INTPOINT p2);
	void SetAnimation(const char name[]);
	void SetAnimation(int index);
	bool CheckCollision(INTPOINT mp);
	void Pause(float time);

	bool UnitAI(bool newMapTile);
	void Attack(MAPOBJECT* _target);
	void ConstructBuilding(int buildToPlace, INTPOINT pos);
	bool isDead();
	void Damage(int dmg, MAPOBJECT* attacker);
	void Heal();

private:

	//Animation variables
	float m_time;						//This units animation time
	float m_speed;						//Movement & animation m_speed
	float m_pauseTime;					//Time to pause
	float m_attackTime;
	float m_mana;
	int m_animation;					//Current animation, Run, Still, attack etc.
	D3DXVECTOR3 m_rotation, m_scale;	//Used to build the world matrix
	ID3DXAnimationController* m_pAnimControl;	//Animation control

	//Status variables
	int m_state;						//Unit state
	D3DXVECTOR3 m_staffPos;				//Used by magician
	int m_buildingToPlace;				//Used by worker to build building
	INTPOINT m_buildingPosition;		// -------- " --------------
	PLAYER* m_pPlayer;

	//Movement variables
	INTPOINT m_finalGoal;
	std::vector<INTPOINT> m_path;		//The active path 
	D3DXVECTOR3 m_lastWP, m_nextWP;		//last & next waypoint
	int m_activeWP;						//active waypoint
	bool m_moving;
	float m_movePrc;					// 0.0 - 1.0, used to interpolate between m_lastWP & m_nextWP
};

#endif