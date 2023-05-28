#ifndef RTS_EFFECT
#define RTS_EFFECT

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "shader.h"
#include "skinnedmesh.h"
#include "mapobject.h"

void LoadEffectResources(IDirect3DDevice9* m_pDevice);
void UnloadEffectResources();

class SMOKE;

//Transform Help structure
struct TRANSFORM
{
	TRANSFORM();
	TRANSFORM(D3DXVECTOR3 _pos);
	TRANSFORM(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot);
	TRANSFORM(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot, D3DXVECTOR3 _sca);

	void Init(D3DXVECTOR3 _pos);
	void Init(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot);
	void Init(D3DXVECTOR3 _pos, D3DXVECTOR3 _rot, D3DXVECTOR3 _sca);

	D3DXMATRIX GetWorldMatrix();

	D3DXVECTOR3 m_pos, m_rot, m_sca;
};

//Virtual EFFECT class...
class EFFECT
{
public:
	EFFECT(IDirect3DDevice9* Dev);
	~EFFECT() {}
	virtual void Update(float timeDelta) = 0;
	virtual void Render() = 0;
	virtual bool isDead() = 0;

	void PreRender();
	void PostRender();

	float m_time;
	IDirect3DDevice9* m_pDevice;
	D3DXVECTOR4 m_color;
};

class EFFECT_SPELL : public EFFECT
{
public:
	EFFECT_SPELL(IDirect3DDevice9* Dev, D3DXVECTOR3 _pos);
	void Update(float timeDelta);
	void Render();
	bool isDead();

private:
	TRANSFORM m_t1;
	TRANSFORM m_c[10];
};

class EFFECT_FIREBALL : public EFFECT
{
public:
	EFFECT_FIREBALL(IDirect3DDevice9* Dev, D3DXVECTOR3* bonePos, MAPOBJECT* _target, MAPOBJECT* _attacker);
	void Update(float timeDelta);
	void Render();
	bool isDead();
	D3DXVECTOR3 GetPosition(float m_prc);

private:
	D3DXVECTOR3* m_pSrcBone;
	float m_speed, m_length, m_prc;
	D3DXVECTOR3 origin, dest;
	MAPOBJECT* m_pTarget, * m_pAttacker;
	int m_damage;
	TRANSFORM m_t1;
};

class EFFECT_LENSFLARE : public EFFECT
{
public:

	struct FLARE
	{
		FLARE(D3DXCOLOR _color, float _place, float _scale, int _sourceFlare)
		{
			color = _color;
			place = _place;
			scale = _scale;
			sourceFlare = _sourceFlare;
		}

		D3DXCOLOR color;
		float place;
		float scale;
		int sourceFlare;
	};

	EFFECT_LENSFLARE(IDirect3DDevice9* Dev, int _type, D3DXVECTOR3 _position);
	void Update(float timeDelta);
	void Render();
	bool isDead();

private:
	float m_mainAlpha;
	bool m_inScreen;
	int m_type;
	D3DXVECTOR3 m_position;
	std::vector<FLARE> m_flares;
};

class EFFECT_FIRE : public EFFECT
{
public:
	EFFECT_FIRE(IDirect3DDevice9* Dev, D3DXVECTOR3 pos, D3DXVECTOR3 scale);
	~EFFECT_FIRE();
	void Update(float timeDelta);
	void Render();
	bool isDead();
	void PreRender();
	void PostRender();
	void Kill();

private:
	D3DXVECTOR3 m_targetScale;
	TRANSFORM m_t1;
	D3DXVECTOR3 m_camEye;
	SMOKE* m_pSmoke;
	bool m_dead;
};

class EFFECT_SELECTED : public EFFECT
{
public:
	EFFECT_SELECTED(IDirect3DDevice9* Dev);
	void Update(float timeDelta);
	void Render();
	bool isDead();
	void Set(float health, D3DXVECTOR3 pos, float rot, float scale);

	TRANSFORM m_t;
};

class EFFECT_TRAINING : public EFFECT
{
public:
	EFFECT_TRAINING(IDirect3DDevice9* Dev);
	void Update(float timeDelta);
	void Render();
	bool isDead();
	void Set(float progress, D3DXVECTOR3 pos);

	TRANSFORM m_t;
};

#endif