#include "heightMap.h"
#include "debug.h"

const DWORD PARTICLE::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
DWORD FtoDword(float f) { return *((DWORD*)&f); }

HEIGHTMAP::HEIGHTMAP(IDirect3DDevice9* Dev, INTPOINT _size)
{
	try
	{
		m_pDevice = Dev;
		m_size = _size;

		// Init sprite
		D3DXCreateSprite(m_pDevice, &m_pSprite);

		//Reset the heightMap to 0
		m_maxHeight = 15.0f;

		//Create a new 2D array with size.x * size.y height values
		m_pHeightMap = new float[m_size.x * m_size.y];

		//Set all heights to 0.0f
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);

		//Set particle vertex buffer and texture to NULL
		m_pVb = NULL;
		m_pHeightMapTexture = NULL;
	}
	catch (...)
	{
		debug.Print("Error in HEIGHTMAP::HEIGHTMAP()");
	}
}

HEIGHTMAP::~HEIGHTMAP()
{
	Release();
}

void HEIGHTMAP::Release()
{
	if (m_pHeightMap != NULL)delete[] m_pHeightMap;
	if (m_pVb != NULL)m_pVb->Release();
	if (m_pSprite != NULL)m_pSprite->Release();
	if (m_pHeightMapTexture != NULL)m_pHeightMapTexture->Release();
}

HRESULT HEIGHTMAP::LoadFromFile(const char fileName[])
{
	try
	{
		//Reset the heightMap to 0.0f
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);

		//Initiate the texture variables
		if (m_pHeightMapTexture != NULL)m_pHeightMapTexture->Release();
		m_pHeightMapTexture = NULL;

		//Load the texture (and scale it to our heightMap size)
		if (FAILED(D3DXCreateTextureFromFileEx(m_pDevice, fileName, m_size.x, m_size.y, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_L8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			NULL, NULL, NULL, &m_pHeightMapTexture)))return E_FAIL;

		//Lock the texture
		D3DLOCKED_RECT sRect;
		m_pHeightMapTexture->LockRect(0, &sRect, NULL, NULL);
		BYTE* bytes = (BYTE*)sRect.pBits;

		//Extract height values from the texture
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				BYTE* b = bytes + y * sRect.Pitch + x;
				BYTE c = *b;
				float f = (c / 255.0f) * m_maxHeight;
				m_pHeightMap[x + y * m_size.x] = f;
				//m_pHeightMap[x + y * m_size.x] = (*b / 255.0f) * m_maxHeight;
			}

		//Unlock the texture
		m_pHeightMapTexture->UnlockRect(0);
	}
	catch (...)
	{
		debug.Print("Error in HEIGHTMAP::LoadFromFile()");
	}

	return S_OK;
}

HRESULT HEIGHTMAP::CreateParticles()
{
	try
	{
		//Don't worry about this code snippet now... we'll 
		//cover particles in greater detail in Chapter 12.

		if (m_pVb != NULL)
		{
			m_pVb->Release();
			m_pVb = NULL;
		}

		if (FAILED(m_pDevice->CreateVertexBuffer(m_size.x * m_size.y * sizeof(PARTICLE), D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY, PARTICLE::FVF, D3DPOOL_DEFAULT, &m_pVb, 0)))
			debug.Print("Failed to create particle vertex buffer");

		PARTICLE* v = NULL;
		m_pVb->Lock(0, 0, (void**)&v, D3DLOCK_DISCARD);

		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				float prc = m_pHeightMap[x + y * m_size.x] / m_maxHeight;
				int red = (int)(255 * prc);
				int green = (int)(255 * (1.0f - prc));

				v->color = D3DCOLOR_ARGB(255, red, green, 0);
				v->position = D3DXVECTOR3(x, m_pHeightMap[x + y * m_size.x], -y);
				v++;
			}

		m_pVb->Unlock();

	}
	catch (...)
	{
		debug.Print("Error in HEIGHTMAP::CreateParticles()");
		return E_FAIL;
	}

	return S_OK;
}

void HEIGHTMAP::Render()
{
	try
	{
		if (m_pVb != NULL)
		{
			m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, true);

			m_pDevice->SetRenderState(D3DRS_POINTSIZE, FtoDword(0.7f));
			m_pDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDword(0.0f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_A, 0);
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_B, FtoDword(0.0f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_C, FtoDword(1.0f));
			m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

			m_pDevice->SetTexture(0, NULL);
			m_pDevice->SetFVF(PARTICLE::FVF);
			m_pDevice->SetStreamSource(0, m_pVb, 0, sizeof(PARTICLE));
			m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_size.x * m_size.y);
		}

		if (m_pSprite != NULL)
		{
			m_pSprite->Begin(0);
			D3DXVECTOR3 pos = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
			m_pSprite->Draw(m_pHeightMapTexture, NULL, NULL, &pos, 0xffffffff);
			m_pSprite->End();
		}
	}
	catch (...)
	{
		debug.Print("Error in HEIGHTMAP::Render()");
	}
}