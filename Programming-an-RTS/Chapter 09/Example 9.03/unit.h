#pragma once

#include "skinnedMesh.h"
#include "mapObject.h"

void LoadUnitResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadUnitResources();


class UNIT : public MAPOBJECT {
	friend class APPLICATION;
	//Animation variables
	float _time;		//this units animation time
	float _speed;		//movement & animation speed
	float _pauseTime;	//time to pause
	int	  _animation;	//current animation, Run, Still, attack, etc
	vec3  _rotation;	//used to build world matrix
	vec3  _scale;		//
	//Movement variables
	INTPOINT _finalGoal;
	std::vector<INTPOINT> _path;	//the active path
	vec3 _lastWP;				//last waypoint
	vec3 _nextWP;				//next waypoint
	int _activeWP;
	bool _moving;
	float _movePrc;				//0.0-1.0, used to interpolate between lastWP and nextWP
public:
	UNIT(int type, int team, INTPOINT mp, TERRAIN* terrain);
	virtual ~UNIT();
	//Abstract functions declared in MAPOBJECT
	void Render(Renderer::Shader* pshader)override;
	void Update(float deltaTime)override;
	BBOX GetBoundingBox()override;
	mat4 GetWorldMatrix()override;

	//Specific UNIT functions
	void Goto(INTPOINT mp, bool considerUnits, bool finalGoal);	//order unit to mp
	void MoveUnit(INTPOINT to);
	vec3 GetDirection(INTPOINT p1, INTPOINT p2);
	void SetAnimation(const char* name);
	void SetAnimation(int index);
	bool CheckCollision(INTPOINT mp);
	void Pause(float time);
	virtual Renderer::Shader* GetShader()override;
	Mesh::AnimationController* UNIT::GetAnimationController()const;
};