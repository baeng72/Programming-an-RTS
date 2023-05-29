#ifndef _RTS_PARTICLES_
#define _RTS_PARTICLES_

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "shader.h"
#include "effect.h"

void LoadParticleResources(IDirect3DDevice9* Dev);
void UnloadParticleResources();

struct PARTICLE
{
	D3DXVECTOR3 position, velocity, acceleration;
	float time_to_live;
	D3DXCOLOR color;
	bool m_dead;
};

class PARTICLE_SYSTEM : public EFFECT
{
public:
	PARTICLE_SYSTEM(IDirect3DDevice9* Dev);
	~PARTICLE_SYSTEM();
	void Update(float timeDelta);
	void Render();
	bool isDead();

	void RenderBatch(int start, int batchSize);
	void PreRender();
	void PostRender();
	DWORD FtoDword(float f) { return *((DWORD*)&f); }

	std::vector<PARTICLE*> m_particles;
	IDirect3DTexture9* m_pTexture;
	DWORD m_blendMode;
	float m_particleSize;
};

class MAGIC_SHOWER : public PARTICLE_SYSTEM
{
public:
	MAGIC_SHOWER(IDirect3DDevice9* Dev, int noParticles, D3DXVECTOR3 _origin);
	void Update(float timeDelta);
	bool isDead();
private:
	D3DXVECTOR3 m_origin;
};

class SMOKE : public PARTICLE_SYSTEM
{
public:
	SMOKE(IDirect3DDevice9* Dev, int noParticles, D3DXVECTOR3 _origin);
	void Update(float timeDelta);
	bool isDead();
	void Kill() { m_dead = true; }

private:
	D3DXVECTOR3 m_origin;
	bool m_dead;
	int m_numAlive;
};

#endif