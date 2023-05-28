#include "mapObject.h"
#include "unit.h"
#include "building.h"
#include "groupAi.h"

//Line interface to draw m_selected units/buildings with
ID3DXLine* line = NULL;

//variables used for fog-of-war
IDirect3DTexture9* sightTexture = NULL;
ID3DXMesh* sightMesh = NULL;
D3DMATERIAL9 sightMtrl;
EFFECT_SELECTED* selectedEffect = NULL;

struct SIGHTVertex
{
	SIGHTVertex() {}
	SIGHTVertex(D3DXVECTOR3 pos, D3DXVECTOR2 _uv)
	{
		position = pos;
		uv = _uv;
	}

	D3DXVECTOR3 position;
	D3DXVECTOR2 uv;

	static const DWORD FVF;
};

const DWORD SIGHTVertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

void LoadMapObjectResources(IDirect3DDevice9* m_pDevice)
{
	D3DXCreateLine(m_pDevice, &line);

	//Sight texture
	m_pDevice->CreateTexture(64, 64, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &sightTexture, NULL);

	D3DLOCKED_RECT sRect;
	sightTexture->LockRect(0, &sRect, NULL, NULL);
	BYTE* bytes = (BYTE*)sRect.pBits;
	memset(bytes, 0, sRect.Pitch * 64);

	float intensity = 1.3f;
	D3DXVECTOR2 center = D3DXVECTOR2(32.0f, 32.0f);

	for (int y = 0; y < 64; y++)
		for (int x = 0; x < 64; x++)
		{
			D3DXVECTOR2 vec2 = (center - D3DXVECTOR2(x, y));
			float d = D3DXVec2Length(&vec2);
			int value = ((32.0f - d) / 32.0f) * 255.0f * intensity;
			if (value < 0)value = 0;
			if (value > 255)value = 255;
			bytes[x + y * sRect.Pitch] = value;
		}
	sightTexture->UnlockRect(0);

	//D3DXSaveTextureToFile("sightTexture.bmp", D3DXIFF_BMP, sightTexture, NULL);

	//Calculate sight mesh (a simple quad)
	D3DXCreateMeshFVF(2, 4, D3DXMESH_MANAGED, SIGHTVertex::FVF, m_pDevice, &sightMesh);

	//Create 4 vertices
	SIGHTVertex* v = 0;
	sightMesh->LockVertexBuffer(0, (void**)&v);
	v[0] = SIGHTVertex(D3DXVECTOR3(-1, 0, 1), D3DXVECTOR2(0, 0));
	v[1] = SIGHTVertex(D3DXVECTOR3(1, 0, 1), D3DXVECTOR2(1, 0));
	v[2] = SIGHTVertex(D3DXVECTOR3(-1, 0, -1), D3DXVECTOR2(0, 1));
	v[3] = SIGHTVertex(D3DXVECTOR3(1, 0, -1), D3DXVECTOR2(1, 1));
	sightMesh->UnlockVertexBuffer();

	//Create 2 faces
	WORD* indices = 0;
	sightMesh->LockIndexBuffer(0, (void**)&indices);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 1; indices[4] = 3; indices[5] = 2;
	sightMesh->UnlockIndexBuffer();

	//Set Attributes for the 2 faces
	DWORD* att = 0;
	sightMesh->LockAttributeBuffer(0, &att);
	att[0] = 0; att[1] = 0;
	sightMesh->UnlockAttributeBuffer();

	//Sight MTRL
	memset(&sightMtrl, 0, sizeof(D3DMATERIAL9));
	sightMtrl.Diffuse = sightMtrl.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	//Selected Effect
	selectedEffect = new EFFECT_SELECTED(m_pDevice);
}

void UnloadMapObjectResources()
{
	if (line)line->Release();
	line = NULL;

	if (sightTexture)sightTexture->Release();
	sightTexture = NULL;

	if (sightMesh)sightMesh->Release();
	sightMesh = NULL;
}

INTPOINT GetScreenPos(D3DXVECTOR3 pos, IDirect3DDevice9* m_pDevice)
{
	D3DXVECTOR3 screenPos;
	D3DVIEWPORT9 Viewport;
	D3DXMATRIX Projection, View, World;

	m_pDevice->GetViewport(&Viewport);
	m_pDevice->GetTransform(D3DTS_VIEW, &View);
	m_pDevice->GetTransform(D3DTS_PROJECTION, &Projection);
	D3DXMatrixIdentity(&World);
	D3DXVec3Project(&screenPos, &pos, &Viewport, &Projection, &View, &World);

	return INTPOINT(screenPos.x, screenPos.y);
}

//////////////////////////////////////////////////////////////////////////////////
//								MapObject										//
//////////////////////////////////////////////////////////////////////////////////

MAPOBJECT::MAPOBJECT()
{
	//Sets all variables to 0, NULL or False
	m_isBuilding = false;
	m_pTerrain = NULL;
	m_hp = m_hpMax = 0;
	m_range = m_damage = 0;
	m_sightRadius = 0.0f;
	m_team = m_type = 0;
	m_selected = m_dead = m_visible = false;
	m_pTarget = NULL;
	m_pDevice = NULL;
	m_pGroup = NULL;
}

RECT MAPOBJECT::GetMapRect(int border)
{
	if (border < 0)border = 0;

	RECT mr = { m_mappos.x - border,
			   m_mappos.y - border,
			   m_mappos.x + m_mapsize.x + border,
			   m_mappos.y + m_mapsize.y + border };

	//Clip Rectangle to Terrain border
	if (m_pTerrain != NULL)
	{
		if (mr.left < 0)mr.left = 0;
		if (mr.right >= m_pTerrain->m_size.x)mr.right = m_pTerrain->m_size.x - 1;
		if (mr.top < 0)mr.top = 0;
		if (mr.bottom >= m_pTerrain->m_size.y)mr.bottom = m_pTerrain->m_size.y - 1;
	}

	return mr;
}

INTPOINT MAPOBJECT::GetAttackPos(INTPOINT from)
{
	try
	{
		if (m_pTerrain == NULL || !m_pTerrain->Within(m_mappos))return INTPOINT(-1, -1);

		RECT r = GetMapRect(1);
		RECT mr = GetMapRect(0);

		INTPOINT bestDest(-1, -1);
		float dist = 10000.0f;

		int loop = 0;

		//Find the closest available attacking position
		for (int y = r.top; y <= r.bottom; y++)
			for (int x = r.left; x <= r.right; x++)
			{
				INTPOINT p(x, y);

				if (!p.inRect(mr))
				{
					MAPTILE* tile = m_pTerrain->GetTile(p);
					float d = from.Distance(p);

					if (tile != NULL && tile->m_pMapObject == NULL && d < dist)
					{
						dist = d;
						bestDest = p;
					}
				}

				if (loop++ > 1000)
				{
					debug.Print("Loop > 1000, MAPOBJECT::GetAttackPos()");
					debug << "R Left: " << r.left << ", Right: " << r.right << ", Top: " << r.top << ", Bottom: " << r.bottom << "\n";
					debug << "Mappos: " << m_mappos.x << ", " << m_mappos.y << "\n";
					return INTPOINT(-1, -1);
				}
			}

		return bestDest;
	}
	catch (...)
	{
		return INTPOINT(-1, -1);
	}
}

void MAPOBJECT::PaintSelected(float time)
{
	if (m_isBuilding)
	{
		BUILDING* build = (BUILDING*)this;
		if (build->m_training)
			build->m_pTrainingEffect->Render();
	}

	if (!m_selected || !selectedEffect)return;

	if (!m_isBuilding)
	{
		selectedEffect->Set(m_hp / (float)m_hpMax,
			m_position + D3DXVECTOR3(0.0f, 0.2f, 0.0f),
			time,
			1.2f);
	}
	else
	{
		selectedEffect->Set(m_hp / (float)m_hpMax,
			m_position + D3DXVECTOR3(0.0f, 1.2f, 0.0f),
			time,
			m_mapsize.x + m_mapsize.y + 2.0f);
	}

	selectedEffect->Render();
}

void MAPOBJECT::RenderSightMesh()
{
	if (m_pDevice == NULL || sightTexture == NULL || sightMesh == NULL)return;

	//Set world transformation matrix
	D3DXMATRIX world, pos, sca;

	//Position the mesh at the center of the map object
	D3DXMatrixTranslation(&pos, m_position.x, m_position.y, m_position.z);

	//Scale the mesh to the sight radius of the mapobject (XZ plane)
	D3DXMatrixScaling(&sca, m_sightRadius, 1.0f, m_sightRadius);

	D3DXMatrixMultiply(&world, &sca, &pos);
	m_pDevice->SetTransform(D3DTS_WORLD, &world);

	//Set texture and material
	m_pDevice->SetTexture(0, sightTexture);
	m_pDevice->SetMaterial(&sightMtrl);

	//Draw the sight mesh
	sightMesh->DrawSubset(0);
}

std::vector<MAPOBJECT*> MAPOBJECT::GetTargetsWithinRange(int theRange)
{
	std::vector<MAPOBJECT*> enemies;

	try
	{
		if (m_pTerrain == NULL)return enemies;

		RECT r = GetMapRect(theRange);

		for (int y = r.top; y <= r.bottom; y++)
			for (int x = r.left; x <= r.right; x++)
			{
				MAPTILE* tile = m_pTerrain->GetTile(x, y);

				if (tile != NULL && tile->m_pMapObject != NULL)
					if (tile->m_pMapObject->m_team != m_team && !tile->m_pMapObject->m_dead)
						if (m_mappos.Distance(INTPOINT(x, y)) <= theRange || m_isBuilding)
							enemies.push_back(tile->m_pMapObject);
			}
	}
	catch (...)
	{
		debug.Print("Error in MAPOBJECT::GetTargetsWithinRange()");
	}

	return enemies;
}

MAPOBJECT* MAPOBJECT::BestTargetToAttack(std::vector<MAPOBJECT*>& enemies)
{
	if (enemies.empty())return NULL;

	int lowestCost = 10000;
	MAPOBJECT* bestTarget = NULL;

	try
	{
		for (int i = 0; i < enemies.size(); i++)
			if (enemies[i] != NULL && !enemies[i]->m_dead)
			{
				INTPOINT attackPos = enemies[i]->GetAttackPos(m_mappos);

				if (m_pTerrain->Within(attackPos) && m_pTerrain->PositionAccessible(this, attackPos))
				{
					int cost = enemies[i]->m_hp + m_mappos.Distance(enemies[i]->m_mappos) * 30;

					if (cost < lowestCost)
					{
						bestTarget = enemies[i];
						lowestCost = cost;
					}
				}
			}
	}
	catch (...)
	{
		debug.Print("Error in MAPOBJECT::BestTargetToAttack()");
	}

	return bestTarget;
}