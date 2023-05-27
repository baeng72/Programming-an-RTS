#include "heightMap.h"
#include "debug.h"

const DWORD PARTICLE::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

DWORD FtoDword(float f) { return *((DWORD*)&f); }


float Noise(int x)
{
	x = (x << 13) ^ x;
	return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float CosInterpolate(float v1, float v2, float a)
{
	float angle = a * D3DX_PI;
	float prc = (1.0f - cos(angle)) * 0.5f;
	return  v1 * (1.0f - prc) + v2 * prc;
}

HEIGHTMAP::HEIGHTMAP(IDirect3DDevice9* Dev, INTPOINT _size)
{
	try
	{
		m_pDevice = Dev;
		m_size = _size;

		// Init m_pSprite
		D3DXCreateSprite(m_pDevice, &m_pSprite);

		//Reset the heightMap to 0
		m_maxHeight = 15.0f;
		m_pHeightMap = new float[m_size.x * m_size.y];
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);

		m_selRect.top = m_selRect.left = m_size.x / 2 - 5;
		m_selRect.bottom = m_selRect.right = m_size.x / 2 + 5;

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

HRESULT HEIGHTMAP::LoadFromFile(char fileName[])
{
	try
	{
		//Reset the heightMap to 0
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);

		//Initiate the texture variables
		if (m_pHeightMapTexture != NULL)m_pHeightMapTexture->Release();
		m_pHeightMapTexture = NULL;
		D3DXIMAGE_INFO info;

		//Load the texture (and scale it to our heightMap m_size)
		if (FAILED(D3DXCreateTextureFromFileEx(m_pDevice, fileName, m_size.x, m_size.y, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_L8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			NULL, &info, NULL, &m_pHeightMapTexture)))return E_FAIL;

		//Lock the texture
		D3DLOCKED_RECT sRect;
		m_pHeightMapTexture->LockRect(0, &sRect, NULL, NULL);
		BYTE* bytes = (BYTE*)sRect.pBits;

		//Extract height values from the texture
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				BYTE* b = bytes + y * sRect.Pitch + x;
				m_pHeightMap[x + y * m_size.x] = ((float)*b / 255.0f) * m_maxHeight;
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

HRESULT HEIGHTMAP::CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves)
{
	if (m_pHeightMapTexture != NULL) { m_pHeightMapTexture->Release(); m_pHeightMapTexture = NULL; }
	m_pDevice->CreateTexture(m_size.x, m_size.y, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &m_pHeightMapTexture, NULL);

	D3DLOCKED_RECT lock;
	m_pHeightMapTexture->LockRect(0, &lock, NULL, NULL);

	//For each map node
	for (int y = 0; y < m_size.y; y++)
		for (int x = 0; x < m_size.x; x++)
		{
			//Scale x & y to the range of 0.0 - m_size
			float xf = ((float)x / (float)m_size.x) * noiseSize;
			float yf = ((float)y / (float)m_size.y) * noiseSize;

			float total = 0;

			// For each octave
			for (int i = 0; i < octaves; i++)
			{
				//Calculate frequency and amplitude (different for each octave)
				float freq = pow(2, i);
				float amp = pow(persistence, i);

				//Calculate the x,y noise coordinates
				float tx = xf * freq;
				float ty = yf * freq;
				int tx_int = tx;
				int ty_int = ty;

				//Calculate the fractions of x & y
				float fracX = tx - tx_int;
				float fracY = ty - ty_int;

				//Get the noise of this octave for each of these 4 points
				float v1 = Noise(tx_int + ty_int * 57 + seed);
				float v2 = Noise(tx_int + 1 + ty_int * 57 + seed);
				float v3 = Noise(tx_int + (ty_int + 1) * 57 + seed);
				float v4 = Noise(tx_int + 1 + (ty_int + 1) * 57 + seed);

				//Smooth in the X-axis
				float i1 = CosInterpolate(v1, v2, fracX);
				float i2 = CosInterpolate(v3, v4, fracX);

				//Smooth in the Y-axis
				total += CosInterpolate(i1, i2, fracY) * amp;
			}

			int b = 128 + total * 128.0f;
			if (b < 0)b = 0;
			if (b > 255)b = 255;

			BYTE* bDest = (BYTE*)lock.pBits;
			bDest += y * lock.Pitch + x;
			*bDest = b;

			//Save to heightMap
			m_pHeightMap[x + y * m_size.x] = ((float)b / 255.0f) * m_maxHeight;
		}

	m_pHeightMapTexture->UnlockRect(0);

	return S_OK;
}

void HEIGHTMAP::RaiseTerrain(RECT r, float f)
{
	for (int y = r.top; y <= r.bottom; y++)
		for (int x = r.left; x <= r.right; x++)
		{
			m_pHeightMap[x + y * m_size.x] += f;
			if (m_pHeightMap[x + y * m_size.x] < -m_maxHeight)m_pHeightMap[x + y * m_size.x] = -m_maxHeight;
			if (m_pHeightMap[x + y * m_size.x] > m_maxHeight)m_pHeightMap[x + y * m_size.x] = m_maxHeight;
		}

	CreateParticles();
}

void HEIGHTMAP::SmoothTerrain()
{
	//Create temporary heightmap
	float* hm = new float[m_size.x * m_size.y];
	memset(hm, 0, sizeof(float) * m_size.x * m_size.y);

	for (int y = 0; y < m_size.y; y++)
		for (int x = 0; x < m_size.x; x++)
		{
			float totalHeight = 0.0f;
			int noNodes = 0;

			//Add all neighboring heights together and use the average
			for (int y1 = y - 1; y1 <= y + 1; y1++)
				for (int x1 = x - 1; x1 <= x + 1; x1++)
					if (x1 >= 0 && x1 < m_size.x && y1 >= 0 && y1 < m_size.y)
					{
						totalHeight += m_pHeightMap[x1 + y1 * m_size.x];
						noNodes++;
					}

			hm[x + y * m_size.x] = totalHeight / (float)noNodes;
		}

	//Replace old heightmap with smoothed heightmap
	delete[] m_pHeightMap;
	m_pHeightMap = hm;

	CreateParticles();

	Sleep(500);
}

void HEIGHTMAP::MoveRect(int dir)
{
	if (dir == LEFT) { m_selRect.left--; m_selRect.right--; }
	if (dir == RIGHT) { m_selRect.left++; m_selRect.right++; }
	if (dir == UP) { m_selRect.top--; m_selRect.bottom--; }
	if (dir == DOWN) { m_selRect.top++; m_selRect.bottom++; }

	Sleep(100);
	CreateParticles();
}

HRESULT HEIGHTMAP::CreateParticles()
{
	try
	{
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
				if (prc < 0.0f)prc = -prc;
				int red = 255 * prc;
				int green = 255 * (1.0f - prc);

				if (x >= m_selRect.left && x <= m_selRect.right && y >= m_selRect.top && y <= m_selRect.bottom)
					v->color = D3DCOLOR_ARGB(255, 0, 0, 255);
				else v->color = D3DCOLOR_ARGB(255, red, green, 0);

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