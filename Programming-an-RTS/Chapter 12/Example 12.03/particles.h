#pragma once

#include <common.h>
#include "effect.h"

void LoadParticleResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager);
void UnloadParticleResources();

struct PARTICLE {
	vec3 position;
	vec3 velocity;
	vec3 acceleration;
	float time_to_live;
	Color color;
	bool dead;
};

class PARTICLE_SYSTEM : public EFFECT {
protected:
	float _particleSize;	
	std::vector<PARTICLE> _particles;
	Renderer::Texture* _pTexture;
public:
	PARTICLE_SYSTEM(Renderer::RenderDevice* pdevice);
	~PARTICLE_SYSTEM();
	void Update(float delta)override;
	void Render(mat4&matVP)override;
	bool isDead()override;
	void RenderBatch(int start, int size);
	void PreRender(mat4&matVP);
	void PostRender();
};

class MAGIC_SHOWER : public PARTICLE_SYSTEM {
	vec3 _origin;
public:
	MAGIC_SHOWER(Renderer::RenderDevice* pdevice, int noParticles, vec3 origin_);
	~MAGIC_SHOWER();
	void Update(float delta) override;
	bool isDead()override;
};