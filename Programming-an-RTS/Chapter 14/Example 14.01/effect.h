#pragma once
#include <common.h>
#include "skinnedMesh.h"
#include "mapObject.h"

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
	virtual void Render(mat4&matProj,mat4&matView) = 0;
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
	void Render(mat4&matProj,mat4&matView)override;
	bool isDead()override;
};

class EFFECT_FIREBALL : public EFFECT {
	
	float _speed;			//speed of fireball
	float _length;			//distance to target
	float _prc;				//percentage of path complete
	TRANSFORM _t1;		//fireball transformation
	vec3 _origin;
	vec3 _dest;
	MAPOBJECT* _pTarget;
	MAPOBJECT* _pAttacker;
	vec3& _pos;
	int _damage;
public:
	EFFECT_FIREBALL(Renderer::RenderDevice* pdevice, vec3&pos, MAPOBJECT*ptarget,MAPOBJECT*pattacker);
	void Update(float delta);
	void Render(mat4& matProj,mat4&matView);
	bool isDead();
	vec3 GetPosition(float p);
};

class EFFECT_LENSFLARE : public EFFECT {
public:
	struct FLARE {
		Color _color;		//Flare color
		float _place;		//position along the ray
		float _scale;		//size of the flare
		int _sourceFlare;	//which flare from the texture to use
		FLARE(Color color, float place, float scale, int sourceFlare) {
			_color = color;
			_place = place;
			_scale = scale;
			_sourceFlare = sourceFlare;
		}
	};

private:
	float _mainAlpha;		//overall alpha
	bool _inScreen;			//inside bounds or screen?
	int _type;				//Flare type
	vec3 _position;			//world position of light source
	std::vector<FLARE> _flares;
	vec4 _viewport;
public:
	EFFECT_LENSFLARE(Renderer::RenderDevice* pdevice, int type, vec3 position);
	void Update(float delta)override;
	void Render(mat4& matProj, mat4& matView)override;
	bool isDead()override;

};
class SMOKE;
class EFFECT_FIRE : public EFFECT {
	TRANSFORM _t1;
	vec3 _camEye;
	SMOKE* _pSmoke;
	bool _dead;
public:
	EFFECT_FIRE(Renderer::RenderDevice* pdevice, vec3 pos, vec3 scale);
	~EFFECT_FIRE();
	void Update(float delta)override;
	void Render(mat4& matProj, mat4& matView)override;
	bool isDead()override;
	void PreRender(mat4&matView);
	void PostRender();
	void Kill();
};

class EFFECT_SELECTED : public EFFECT {
	TRANSFORM _t;
public:
	EFFECT_SELECTED(Renderer::RenderDevice* pdevice);
	void Update(float delta)override;
	void Render(mat4& matProj, mat4& matView)override;
	bool isDead()override;
	void Set(float progress, vec3 pos);
};

class EFFECT_TRAINING : public EFFECT {
	TRANSFORM _t;
public:
	EFFECT_TRAINING(Renderer::RenderDevice* pdevice);
	void Update(float delta)override;
	void Render(mat4& matProj, mat4& matView)override;
	bool isDead()override;
	void Set(float progress, vec3 pos);
};