#ifndef RTS_EFFECT
#define RTS_EFFECT

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "shader.h"

void LoadEffectResources(IDirect3DDevice9* m_pDevice);
void UnloadEffectResources();

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

	IDirect3DDevice9* m_pDevice;
	float m_time;
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
	TRANSFORM m_t1;		//Rune billboard transform
	TRANSFORM m_c[10];	//Glow billboards
};

#endif