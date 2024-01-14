#pragma once
#include <common.h>
#include "skinnedMesh.h"
void LoadEffectResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager);
void UnloadEffectResources();

//Transform help structure
struct TRANSFORM {
	vec3 _pos;
	vec3 _rot;
	vec3 _sca;

	mat4 GetWorldMatrix();
	TRANSFORM();
	TRANSFORM(vec3 pos_);
	TRANSFORM(vec3 pos_, vec3 rot_);
	TRANSFORM(vec3 pos_, vec3 rot_, vec3 sca_);

	void Init(vec3 pos_);
	void Init(vec3 pos_, vec3 rot_);
	void Init(vec3 pos_, vec3 rot_, vec3 sca_);
};

//virtual EFFECT class...
struct EFFECT {
	Renderer::RenderDevice* _pdevice;
	float _time;
	Color _color;
	EFFECT(Renderer::RenderDevice* pdevice);
	virtual ~EFFECT() = default;
	virtual void Update(float delta) = 0;
	virtual void Render(mat4&matVP) = 0;
	virtual bool isDead() = 0;
	void PreRender(mat4&matVP);
	void PostRender();
};

class EFFECT_SPELL : public EFFECT {
	TRANSFORM _t1;		//Rune billboard transform
	TRANSFORM _c[10];	//Glow billboards
public:
	EFFECT_SPELL(Renderer::RenderDevice* pdevice, vec3 pos);
	void Update(float delta)override;
	void Render(mat4&matVP)override;
	bool isDead()override;
};

class EFFECT_FIREBALL : public EFFECT {
	SKINNEDMESH &_skinnedMesh;		//bone to follow during phase 1
	int _boneID;
	float _speed;			//speed of fireball
	float _length;			//distance to target
	float _prc;				//percentage of path complete
	TRANSFORM _t1;		//fireball transformation
	vec3 _origin;
	vec3 _dest;
	mat4 _loc;
public:
	EFFECT_FIREBALL(Renderer::RenderDevice* pdevice, mat4& loc,SKINNEDMESH&skinnedMesh,int boneID, vec3 dest);
	void Update(float delta);
	void Render(mat4& matVP);
	bool isDead();
	vec3 GetPosition(float p);
};