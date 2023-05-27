#include "city.h"

CITY::CITY()
{

}

void CITY::Init(INTPOINT _size)
{
	m_size = _size;

	m_objects.clear();

	for (int y = 0; y < m_size.y; y++)
		for (int x = 0; x < m_size.x; x++)
		{
			//Add tile
			m_objects.push_back(OBJECT(TILE, D3DXVECTOR3(x * TILE_SIZE, 0.0f, y * -TILE_SIZE), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f)));

			//Add house
			float sca_xz = rand() % 100 / 1000.0f - 0.05f;
			float sca_y = rand() % 500 / 1000.0f - 0.25f;
			int rotation = rand() % 4;
			int house = rand() % 2 + 1;
			if (x % 3 == 0 && y % 3 == 0)house = PARK;
			auto pos = D3DXVECTOR3(x * TILE_SIZE, 0.0f, y * -TILE_SIZE);
			if (m_objects.size() == 603) {
				int z = 0;
			}
			m_objects.push_back(OBJECT(house, pos,
				D3DXVECTOR3(0.0f, (D3DX_PI / 2.0f) * rotation, 0.0f),
				D3DXVECTOR3(1.0f + sca_xz, 1.0f + sca_y, 1.0f + sca_xz)));

		}
}

void CITY::Render(CAMERA* cam)
{
	int32_t drawCount = 0;
	for (int i = 0; i < m_objects.size(); i++)
	{
		if (i == 600) {
			int z = 0;
		}
		if (cam == NULL)
		{
			if (m_objects[i].m_rendered) {
				m_objects[i].Render();
				drawCount++;
			}
		}
		//else if(cam->Cull(m_objects[i].m_bBox))			//Box culling
		else if (cam->Cull(m_objects[i].m_bSphere))	//Sphere culling
		{
			m_objects[i].m_rendered = false;
		}
		else
		{
			m_objects[i].Render();
			m_objects[i].m_rendered = true;
			drawCount++;
		}
	}
}

D3DXVECTOR3 CITY::GetCenter()
{
	return D3DXVECTOR3(m_size.x / 2.0f * TILE_SIZE, 0.0f, m_size.y / 2.0f * -TILE_SIZE);
}