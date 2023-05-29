#include "building.h"
#include "groupai.h"
#include "player.h"

std::vector<MESH*> buildingMeshes;

void LoadBuildingResources(IDirect3DDevice9* m_pDevice)
{
	std::vector<std::string> fnames;

	fnames.push_back("meshes/townhall_b.x");
	fnames.push_back("meshes/townhall.x");
	fnames.push_back("meshes/barracks_b.x");
	fnames.push_back("meshes/barracks.x");
	fnames.push_back("meshes/tower_b.x");
	fnames.push_back("meshes/tower.x");

	for (int i = 0; i < fnames.size(); i++)
		buildingMeshes.push_back(new MESH((char*)fnames[i].c_str(), m_pDevice));
}

void UnloadBuildingResources()
{
	for (int i = 0; i < buildingMeshes.size(); i++)
		if (buildingMeshes[i] != NULL)
			delete buildingMeshes[i];

	buildingMeshes.clear();
}

bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* terrain)
{
	if (terrain == NULL)return false;

	BUILDING b(buildType, 0, true, mp, NULL, NULL, false, NULL);
	RECT r = b.GetMapRect(2);

	for (int y = r.top; y <= r.bottom; y++)
		for (int x = r.left; x <= r.right; x++)
		{
			//Building must be within map borders
			if (!terrain->Within(INTPOINT(x, y)))return false;
			MAPTILE* tile = terrain->GetTile(x, y);
			if (tile == NULL)return false;

			//The terrain must be level and walkable
			if (tile->m_type != 0 || !tile->m_walkable)return false;
		}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//								BUILDING										//
//////////////////////////////////////////////////////////////////////////////////

BUILDING::BUILDING(int _type, int _team, bool finished, INTPOINT mp, TERRAIN* _terrain, PLAYER* _player, bool _affectTerrain, IDirect3DDevice9* Dev)
{
	m_type = _type;
	m_team = _team;
	m_mappos = mp;
	m_pTerrain = _terrain;
	m_affectTerrain = _affectTerrain;
	m_pDevice = Dev;
	m_range = m_damage = 0;
	m_isBuilding = true;
	m_deathCountDown = 0.0f;
	m_training = false;
	m_pPlayer = _player;
	m_pTarget = NULL;
	m_pTrainingEffect = NULL;

	if (finished)
	{
		m_buildProgress = 1.0f;
		m_meshInstance.SetMesh(buildingMeshes[m_type * 2 + 1]);
	}
	else
	{
		m_buildProgress = 0.0f;
		m_meshInstance.SetMesh(buildingMeshes[m_type * 2]);
	}

	if (m_type == TOWNHALL)
	{
		m_hp = m_hpMax = 600;
		m_sightRadius = 12;
		m_name = "Townhall";
		m_mapsize.Set(4, 2);
		m_meshInstance.SetScale(D3DXVECTOR3(0.13f, 0.13f, 0.13f));
		m_buildSpeed = 0.05f;
	}
	else if (m_type == BARRACKS)
	{
		m_hp = m_hpMax = 450;
		m_sightRadius = 13;
		m_name = "Barracks";
		m_mapsize.Set(2, 4);
		m_meshInstance.SetScale(D3DXVECTOR3(0.15f, 0.15f, 0.15f));
		m_buildSpeed = 0.07f;
	}
	else if (m_type == TOWER)
	{
		m_hp = m_hpMax = 750;
		m_sightRadius = 20;
		m_name = "Tower";
		m_mapsize.Set(2, 2);
		m_meshInstance.SetScale(D3DXVECTOR3(0.13f, 0.13f, 0.13f));
		m_buildSpeed = 0.04f;
	}

	if (m_pTerrain != NULL)
		m_position = m_pTerrain->GetWorldPos(m_mappos) + D3DXVECTOR3(m_mapsize.x / 2.0f, 0.0f, -m_mapsize.y / 2.0f);
	else m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_meshInstance.SetPosition(m_position);

	m_BBox = m_meshInstance.GetBoundingBox();
	m_BBox.max -= D3DXVECTOR3(0.2f, 0.2f, 0.2f);
	m_BBox.min += D3DXVECTOR3(0.2f, 0.2f, 0.2f);

	//Update the tiles of the m_pTerrain
	if (m_pTerrain != NULL && m_affectTerrain)
	{
		RECT mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++)
			for (int x = mr.left; x <= mr.right; x++)
			{
				MAPTILE* tile = m_pTerrain->GetTile(x, y);
				if (tile != NULL)
				{
					tile->m_walkable = false;
					tile->m_pMapObject = this;
				}
			}

		m_pTerrain->m_updateSight = true;
		RECT r = GetMapRect(1);
		m_pTerrain->UpdatePathfinding(&r);
		m_pTrainingEffect = new EFFECT_TRAINING(m_pDevice);
	}

	//Set m_fires positions and scales
	if (m_type == TOWNHALL)
	{
		m_firePos[0] = m_position + D3DXVECTOR3(0.0f, 3.0f, 1.0f);
		m_firePos[1] = m_position + D3DXVECTOR3(-2.0f, 1.2f, -0.4f);
		m_firePos[2] = m_position + D3DXVECTOR3(2.0f, 0.0f, -1.0f);
		m_fireScale[0] = D3DXVECTOR3(3.0f, 4.0f, 3.0f);
		m_fireScale[1] = D3DXVECTOR3(3.0f, 8.0f, 3.0f);
		m_fireScale[2] = D3DXVECTOR3(3.0f, 8.0f, 3.0f);
	}
	else if (m_type == BARRACKS)
	{
		m_firePos[0] = m_position + D3DXVECTOR3(-0.85f, 2.5f, 0.7f);
		m_firePos[1] = m_position + D3DXVECTOR3(0.5f, 0.0f, 0.0f);
		m_firePos[2] = m_position + D3DXVECTOR3(0.0f, 0.0f, -2.0f);
		m_fireScale[0] = D3DXVECTOR3(3.0f, 3.0f, 3.5f);
		m_fireScale[1] = D3DXVECTOR3(2.0f, 2.0f, 3.5f);
		m_fireScale[2] = D3DXVECTOR3(3.0f, 3.0f, 4.0f);
	}
	else if (m_type == TOWER)
	{
		m_firePos[0] = m_position + D3DXVECTOR3(0.0f, 0.0f, 0.85f);
		m_firePos[1] = m_position + D3DXVECTOR3(-0.6f, 3.0f, -0.4f);
		m_firePos[2] = m_position + D3DXVECTOR3(0.0f, 5.0f, 0.0f);
		m_fireScale[0] = D3DXVECTOR3(1.5f, 1.5f, 3.5f);
		m_fireScale[1] = D3DXVECTOR3(1.5f, 1.5f, 2.0f);
		m_fireScale[2] = D3DXVECTOR3(3.0f, 3.0f, 4.0f);
	}
}

BUILDING::~BUILDING()
{
	try
	{
		for (int i = 0; i < m_fires.size(); i++)
			if (m_fires[i] != NULL)
				delete m_fires[i];
		m_fires.clear();

		if (m_pTrainingEffect != NULL)
			delete m_pTrainingEffect;
	}
	catch (...)
	{
		debug.Print("Error in ~BUILDING()");
	}
}

void BUILDING::Render()
{
	if (m_visible)
	{
		if (m_buildProgress < 1.0f)
			m_meshInstance.Render(m_buildProgress);
		else
		{
			m_meshInstance.Render();
		}
	}
}

void BUILDING::RenderFires()
{
	//Render m_fires
	for (int i = 0; i < m_fires.size(); i++)
		if (m_fires[i] != NULL)
			m_fires[i]->Render();
}

void BUILDING::Update(float deltaTime)
{
	try
	{
		if (m_dead)
		{
			m_deathCountDown += deltaTime;
			m_position.y -= deltaTime;
			m_meshInstance.m_pos = m_position;

			//Shrink sightradius when a unit has died
			int oldSr = m_sightRadius;
			if (m_sightRadius > 0.0f)m_sightRadius -= deltaTime * 2.0f;
			if (m_sightRadius < 0.0f)m_sightRadius = 0.0f;
			if (oldSr != (int)m_sightRadius)m_pTerrain->m_updateSight = true;
		}
		else
		{
			//Building building
			if (m_buildProgress < 1.0f)
			{
				m_buildProgress += deltaTime * m_buildSpeed;

				if (m_buildProgress >= 1.0f)
				{
					m_buildProgress = 1.0f;
					m_meshInstance.SetMesh(buildingMeshes[m_type * 2 + 1]);

					//Deploy worker unit
					if (m_pTarget != NULL)
					{
						INTPOINT from(m_mappos.x + rand() % 10 - 5, m_mappos.y + rand() % 10 - 5);
						INTPOINT p = GetAttackPos(from);
						if (!m_pTerrain->Within(p))
							p = m_pTerrain->GetClosestFreeTile(m_mappos, from);

						UNIT* unit = (UNIT*)m_pTarget;
						unit->MoveUnit(p);
						unit->m_position = m_pTerrain->GetWorldPos(p);
						m_pPlayer->m_mapObjects.push_back(unit);
						m_pTarget = NULL;
					}
				}
			}
			else
			{
				if (m_training)
				{
					m_pTrainingEffect->Update(deltaTime);
					float progress = m_trainingTime / m_maxTrainingTime;
					m_pTrainingEffect->Set(progress, m_position + D3DXVECTOR3(0.0f, m_BBox.max.y - m_BBox.min.y + 1.5f, 0.0f));

					m_trainingTime += deltaTime;

					if (m_trainingTime >= m_maxTrainingTime)
					{
						m_training = false;
						INTPOINT from(m_mappos.x + rand() % 10 - 5, m_mappos.y + rand() % 10 - 5);
						INTPOINT p = GetAttackPos(from);
						if (!m_pTerrain->Within(p))
							p = m_pTerrain->GetClosestFreeTile(m_mappos, from);

						UNIT* unit = (UNIT*)m_pPlayer->AddMapObject(m_trainingUnit, p, false, true);
						unit->Goto(m_pTerrain->GetClosestFreeTile(unit->m_mappos, unit->m_mappos), false, true, STATE_MOVING);
					}
				}
			}
		}

		//Update m_fires
		for (int i = 0; i < m_fires.size(); i++)
			if (m_fires[i] != NULL)
				m_fires[i]->Update(deltaTime);
	}
	catch (...)
	{
		debug.Print("Error in BUILDING::Update()");
	}
}

BBOX BUILDING::GetBoundingBox()
{
	return m_BBox;
}

D3DXMATRIX BUILDING::GetWorldMatrix()
{
	return m_meshInstance.GetWorldMatrix();
}

bool BUILDING::isDead()
{
	return m_deathCountDown > 10.0f;
}

void BUILDING::Damage(int dmg, MAPOBJECT* attacker)
{
	if (m_dead)return;

	int oldHP = m_hp;
	m_hp -= dmg;

	//Add m_fires
	int limit[3] = { m_hpMax * 0.75f, m_hpMax * 0.5f, m_hpMax * 0.25f };
	for (int i = 0; i < 3; i++)
		if (m_hp < limit[i] && oldHP >= limit[i])
			m_fires.push_back(new EFFECT_FIRE(m_pDevice, m_firePos[i], m_fireScale[i]));

	if (m_hp <= 0)
	{
		m_hp = 0;
		m_dead = true;

		UNIT* attackUnit = (UNIT*)attacker;
		if (attackUnit != NULL && attackUnit->m_pPlayer != NULL)
			attackUnit->m_pPlayer->m_numKills++;

		//restore the tiles of the terrain
		if (m_pTerrain != NULL && m_affectTerrain)
		{
			RECT mr = GetMapRect(0);

			for (int y = mr.top; y <= mr.bottom; y++)
				for (int x = mr.left; x <= mr.right; x++)
				{
					MAPTILE* tile = m_pTerrain->GetTile(x, y);
					if (tile != NULL)
					{
						tile->m_walkable = true;
						tile->m_pMapObject = NULL;
					}
				}
			RECT r = GetMapRect(1);
			m_pTerrain->UpdatePathfinding(&r);
			m_pTerrain->m_updateSight = true;
		}

		//Kill m_fires
		for (int i = 0; i < m_fires.size(); i++)
			if (m_fires[i] != NULL)
				m_fires[i]->Kill();

		//Leave group
		if (m_pGroup != NULL)
		{
			m_pGroup->RemoveMember(this);
			m_pGroup = NULL;
		}
	}
}

void BUILDING::TrainUnit(int unit)
{
	if (m_pPlayer->money < GetCost(unit, false))return;

	m_training = true;
	m_trainingUnit = unit;
	m_trainingTime = 0.0f;
	m_pPlayer->money -= GetCost(unit, false);

	if (unit == WORKER)m_maxTrainingTime = 15.0f;
	if (unit == SOLDIER)m_maxTrainingTime = 20.0f;
	if (unit == MAGICIAN)m_maxTrainingTime = 30.0f;
}