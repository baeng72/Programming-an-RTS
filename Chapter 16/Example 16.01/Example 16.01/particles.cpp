#include "particles.h"

//Global Particle Textures
IDirect3DTexture9* starTexture = NULL;
IDirect3DTexture9* smokeTexture = NULL;

struct PARTICLE_VERTEX
{
	D3DXVECTOR3 position;
	D3DCOLOR color;
	static const DWORD FVF;
};

const DWORD PARTICLE_VERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

int numParticles = 2048;
IDirect3DVertexBuffer9* particleBuffer = NULL;
DWORD bufferOffset = 0;

void LoadParticleResources(IDirect3DDevice9* Dev)
{
	Dev->CreateVertexBuffer(numParticles * sizeof(PARTICLE_VERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY,
		PARTICLE_VERTEX::FVF, D3DPOOL_DEFAULT, &particleBuffer, NULL);

	//Load textures
	D3DXCreateTextureFromFile(Dev, "textures/star.dds", &starTexture);
	D3DXCreateTextureFromFile(Dev, "textures/smoke.dds", &smokeTexture);
}

void UnloadParticleResources()
{
	if (particleBuffer)
		particleBuffer->Release();
	particleBuffer = NULL;

	//Release textures
	if (starTexture)starTexture->Release();
	if (smokeTexture)smokeTexture->Release();

	starTexture = NULL;
	smokeTexture = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								PARTICLE_SYSTEM												//
//////////////////////////////////////////////////////////////////////////////////////////////

PARTICLE_SYSTEM::PARTICLE_SYSTEM(IDirect3DDevice9* Dev) : EFFECT(Dev)
{
	m_pTexture = NULL;
	m_blendMode = D3DBLEND_ONE;
	m_particleSize = 3.0f;
}

PARTICLE_SYSTEM::~PARTICLE_SYSTEM()
{
	//Delete all particles
	for (int i = 0; i < m_particles.size(); i++)
		delete m_particles[i];
	m_particles.clear();
}

void PARTICLE_SYSTEM::Update(float timeDelta)
{

}

void PARTICLE_SYSTEM::Render()
{
	if (m_particles.empty() || particleBuffer == NULL)return;

	PreRender();

	m_pDevice->SetTexture(0, m_pTexture);
	m_pDevice->SetFVF(PARTICLE_VERTEX::FVF);
	m_pDevice->SetStreamSource(0, particleBuffer, 0, sizeof(PARTICLE_VERTEX));

	int batchSize = 512;

	for (int i = 0; i < m_particles.size(); i += batchSize)
		RenderBatch(i, batchSize);

	PostRender();
}

void PARTICLE_SYSTEM::RenderBatch(int start, int batchSize)
{
	//If we will reach the end of the vertex buffer, start over
	if (bufferOffset + batchSize >= numParticles)bufferOffset = 0;

	//Lock the vertex buffer
	PARTICLE_VERTEX* p = NULL;
	particleBuffer->Lock(bufferOffset * sizeof(PARTICLE_VERTEX),
		batchSize, (void**)&p,
		bufferOffset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);

	int particlesRendered = 0;
	for (int i = start; i < m_particles.size() && i < start + batchSize; i++)
		if (!m_particles[i]->m_dead)
		{
			p->position = m_particles[i]->position;
			p->color = m_particles[i]->color;
			p++;
			particlesRendered++;
		}

	particleBuffer->Unlock();

	//Render batch 
	if (particlesRendered > 0)
		m_pDevice->DrawPrimitive(D3DPT_POINTLIST, bufferOffset, particlesRendered);

	//Increase offset
	bufferOffset += batchSize;
}

bool PARTICLE_SYSTEM::isDead()
{
	return true;
}

void PARTICLE_SYSTEM::PreRender()
{
	m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_POINTSIZE, FtoDword(m_particleSize));
	m_pDevice->SetRenderState(D3DRS_POINTSCALE_A, FtoDword(0.0f));
	m_pDevice->SetRenderState(D3DRS_POINTSCALE_B, FtoDword(0.0f));
	m_pDevice->SetRenderState(D3DRS_POINTSCALE_C, FtoDword(1.0f));
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);

	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, m_blendMode);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	m_pDevice->SetTransform(D3DTS_WORLD, &identity);
}

void PARTICLE_SYSTEM::PostRender()
{
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								MAGIC SHOWER												//
//////////////////////////////////////////////////////////////////////////////////////////////

MAGIC_SHOWER::MAGIC_SHOWER(IDirect3DDevice9* Dev, int noParticles, D3DXVECTOR3 _origin) : PARTICLE_SYSTEM(Dev)
{
	m_origin = _origin;

	//Add initial m_particles
	for (int i = 0; i < noParticles; i++)
	{
		PARTICLE* p = new PARTICLE();
		memset(p, 0, sizeof(PARTICLE));
		p->time_to_live = rand() % 5000 / 1000.0f;
		p->color.a = 0.0f;
		p->acceleration = D3DXVECTOR3(0.0f, -0.5f, 0.0f);
		m_particles.push_back(p);
	}

	m_pTexture = starTexture;
}

void MAGIC_SHOWER::Update(float timeDelta)
{
	for (int i = 0; i < m_particles.size(); i++)
	{
		m_particles[i]->time_to_live -= timeDelta;
		m_particles[i]->velocity += m_particles[i]->acceleration * timeDelta;	//Gravity
		m_particles[i]->position += m_particles[i]->velocity * timeDelta;
		m_particles[i]->color.a = m_particles[i]->time_to_live / 5.0f;

		if (m_particles[i]->time_to_live <= 0.0f)	//Re-spawn
		{
			m_particles[i]->position = m_origin;

			m_particles[i]->velocity = D3DXVECTOR3((rand() % 2000 / 1000.0f) - 1.0f,
				(rand() % 2000 / 1000.0f) - 1.0f,
				(rand() % 2000 / 1000.0f) - 1.0f);

			D3DXVec3Normalize(&m_particles[i]->velocity, &m_particles[i]->velocity);
			m_particles[i]->velocity *= 2.0f;

			m_particles[i]->color = D3DXCOLOR(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1.0f);

			m_particles[i]->time_to_live = rand() % 4000 / 1000.0f + 1.0f;
		}
	}
}

bool MAGIC_SHOWER::isDead()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								SMOKE														//
//////////////////////////////////////////////////////////////////////////////////////////////

SMOKE::SMOKE(IDirect3DDevice9* Dev, int noParticles, D3DXVECTOR3 _origin) : PARTICLE_SYSTEM(Dev)
{
	m_origin = _origin;
	m_numAlive = 1;

	//Add initial particles
	for (int i = 0; i < noParticles; i++)
	{
		PARTICLE* p = new PARTICLE();
		memset(p, 0, sizeof(PARTICLE));
		p->time_to_live = rand() % 5000 / 1000.0f;
		p->m_dead = true;
		p->acceleration = D3DXVECTOR3(-0.07f, 0.05f, 0.02f);		//Sideways wind
		m_particles.push_back(p);
	}

	m_dead = false;
	m_blendMode = D3DBLEND_INVSRCALPHA;
	m_particleSize = 4.0f;
	m_pTexture = smokeTexture;
}

void SMOKE::Update(float timeDelta)
{
	m_numAlive = 0;

	for (int i = 0; i < m_particles.size(); i++)
	{
		if (!m_particles[i]->m_dead)
		{
			m_particles[i]->time_to_live -= timeDelta;
			m_particles[i]->velocity += m_particles[i]->acceleration * timeDelta;	//Wind
			m_particles[i]->position += m_particles[i]->velocity * timeDelta;
			m_numAlive++;
			float aim = m_particles[i]->time_to_live * 0.1f;

			if (m_particles[i]->time_to_live <= 0.0f)	//Re-spawn
			{
				m_particles[i]->color.a -= timeDelta * 0.05f;

				if (m_particles[i]->color.a <= 0.0f)
				{
					m_particles[i]->color.a = 0.0f;
					m_particles[i]->m_dead = true;
				}
			}
			else if (m_particles[i]->color.a < aim)
				m_particles[i]->color.a += timeDelta * 0.015f;
		}

		if (m_particles[i]->m_dead && !m_dead)
		{
			//Random start position
			D3DXVECTOR3 p = D3DXVECTOR3((rand() % 2000 / 1000.0f) - 1.0f,
				(rand() % 2000 / 1000.0f) - 1.0f,
				(rand() % 2000 / 1000.0f) - 1.0f);
			p *= 0.5f;
			p.y -= (rand() % 200) / 1000.0f;

			m_particles[i]->position = m_origin + p;

			//Circular direction
			float angle = (rand() % 6280) / 1000.0f;
			float spread = ((rand() % 1000) / 1000.0f) * 0.4f;

			m_particles[i]->velocity = D3DXVECTOR3(cos(angle) * spread,
				1.0f + ((rand() % 2000 / 1000.0f) - 1.0f) * 0.5f,
				sin(angle) * spread);

			D3DXVec3Normalize(&m_particles[i]->velocity, &m_particles[i]->velocity);
			m_particles[i]->velocity *= 0.5f;

			//Random graydscale color
			float c = rand() % 900 / 1000.0f + 0.1f;
			m_particles[i]->color = D3DXCOLOR(c, c, c, 0.01f);

			m_particles[i]->time_to_live = rand() % 6000 / 1000.0f + 1.0f;

			m_particles[i]->m_dead = false;
		}
	}
}

bool SMOKE::isDead()
{
	return m_numAlive > 0;
}