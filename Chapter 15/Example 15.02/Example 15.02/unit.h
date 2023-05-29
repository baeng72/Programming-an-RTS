#ifndef _RTS_UNIT_
#define _RTS_UNIT_

#include "skinnedmesh.h"
#include "mapObject.h"

void LoadUnitResources(IDirect3DDevice9* m_pDevice);
void UnloadUnitResources();

class UNIT : public MAPOBJECT
{
	friend class APPLICATION;
public:
	UNIT(int _type, int _team, INTPOINT mp, TERRAIN* _terrain, IDirect3DDevice9* Dev);
	~UNIT();

	//Abstract functions declared in MAPOBJECT
	void Render();
	void Update(float deltaTime);
	BBOX GetBoundingBox();
	D3DXMATRIX GetWorldMatrix();

	//Specific UNIT functions
	void MoveTo(INTPOINT mp, bool considerUnits, bool _finalGoal);	//Move unit to mp
	D3DXVECTOR3 GetDirection(INTPOINT p1, INTPOINT p2);
	void SetAnimation(const char name[]);
	void SetAnimation(int index);
	bool CheckCollision(INTPOINT mp);
	void Pause(float time);
	void MoveUnit(INTPOINT to);

private:

	//Animation variables
	float m_time;					//This units animation time
	float m_speed;					//Movement & animation m_speed
	float m_pauseTime;				//Time to pause
	int m_animation;					//Current animation, Run, Still, attack etc.
	D3DXVECTOR3 m_rotation, m_scale;	//Used to build the world matrix
	ID3DXAnimationController* m_pAnimControl;	//Animation control

	//Movement variables
	INTPOINT m_finalGoal;
	std::vector<INTPOINT> m_path;		//The active path 
	D3DXVECTOR3 m_lastWP, m_nextWP;		//last & next waypoint
	int m_activeWP;						//active waypoint
	bool m_moving;
	float m_movePrc;					// 0.0 - 1.0, used to interpolate between m_lastWP & m_nextWP
};

#endif