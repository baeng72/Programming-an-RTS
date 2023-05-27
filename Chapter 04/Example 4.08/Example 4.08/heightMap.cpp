#include "heightMap.h"
#include "debug.h"

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

HEIGHTMAP::HEIGHTMAP(INTPOINT _size, float _maxHeight)
{
	try
	{
		m_size = _size;
		m_fMaxHeight = _maxHeight;

		m_pHeightMap = new float[m_size.x * m_size.y];
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);
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

void HEIGHTMAP::operator*=(const HEIGHTMAP& rhs)
{
	for (int y = 0; y < m_size.y; y++)
		for (int x = 0; x < m_size.x; x++)
		{
			float a = m_pHeightMap[x + y * m_size.x] / m_fMaxHeight;
			float b = 1.0f;
			if (x <= rhs.m_size.x && y <= rhs.m_size.y)
				b = rhs.m_pHeightMap[x + y * m_size.x] / rhs.m_fMaxHeight;

			m_pHeightMap[x + y * m_size.x] = a * b * m_fMaxHeight;
		}
}

void HEIGHTMAP::Release()
{
	if (m_pHeightMap != NULL)delete[] m_pHeightMap;
	m_pHeightMap = NULL;
}

HRESULT HEIGHTMAP::LoadFromFile(IDirect3DDevice9* Device, const char fileName[])
{
	try
	{
		//Reset the heightMap to 0.0f
		memset(m_pHeightMap, 0, sizeof(float) * m_size.x * m_size.y);

		//Initiate the texture variables
		IDirect3DTexture9* heightMapTexture = NULL;
		D3DXIMAGE_INFO info;

		//Load the texture (and scale it to our heightMap m_size)
		if (FAILED(D3DXCreateTextureFromFileEx(Device, fileName, m_size.x, m_size.y, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_L8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			NULL, &info, NULL, &heightMapTexture)))return E_FAIL;

		//Lock the texture
		D3DLOCKED_RECT sRect;
		heightMapTexture->LockRect(0, &sRect, NULL, NULL);
		BYTE* bytes = (BYTE*)sRect.pBits;

		//Extract height values from the texture
		for (int y = 0; y < m_size.y; y++)
			for (int x = 0; x < m_size.x; x++)
			{
				BYTE* b = bytes + y * sRect.Pitch + x;
				m_pHeightMap[x + y * m_size.x] = (*b / 255.0f) * m_fMaxHeight;
			}

		//Unlock the texture
		heightMapTexture->UnlockRect(0);

		//Release texture
		heightMapTexture->Release();
	}
	catch (...)
	{
		debug.Print("Error in HEIGHTMAP::LoadFromFile()");
	}

	return S_OK;
}

HRESULT HEIGHTMAP::CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves)
{
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

			//Save to heightMap
			m_pHeightMap[x + y * m_size.x] = (b / 255.0f) * m_fMaxHeight;
		}

	return S_OK;
}

void HEIGHTMAP::RaiseTerrain(RECT r, float f)
{
	for (int y = r.top; y <= r.bottom; y++)
		for (int x = r.left; x <= r.right; x++)
		{
			m_pHeightMap[x + y * m_size.x] += f;
			if (m_pHeightMap[x + y * m_size.x] < -m_fMaxHeight)m_pHeightMap[x + y * m_size.x] = -m_fMaxHeight;
			if (m_pHeightMap[x + y * m_size.x] > m_fMaxHeight)m_pHeightMap[x + y * m_size.x] = m_fMaxHeight;
		}
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
}

void HEIGHTMAP::Cap(float capHeight)
{
	m_fMaxHeight = 0.0f;

	for (int y = 0; y < m_size.y; y++)
		for (int x = 0; x < m_size.x; x++)
		{
			m_pHeightMap[x + y * m_size.x] -= capHeight;
			if (m_pHeightMap[x + y * m_size.x] < 0.0f)
				m_pHeightMap[x + y * m_size.x] = 0.0f;

			if (m_pHeightMap[x + y * m_size.x] > m_fMaxHeight)
				m_fMaxHeight = m_pHeightMap[x + y * m_size.x];
		}
}