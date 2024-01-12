#pragma once
#include <common.h>

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
	~EFFECT() = default;
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

