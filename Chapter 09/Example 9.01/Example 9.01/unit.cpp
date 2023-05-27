#include "unit.h"

std::vector<SKINNEDMESH*> unitMeshes;

void LoadUnitResources(IDirect3DDevice9* Device)
{
	std::vector<std::string> fnames;

	fnames.push_back("units/drone.x");
	fnames.push_back("units/soldier.x");
	fnames.push_back("units/magician.x");

	for (int i = 0; i < fnames.size(); i++)
	{
		SKINNEDMESH* newMesh = new SKINNEDMESH();
		newMesh->Load((char*)fnames[i].c_str(), Device);
		unitMeshes.push_back(newMesh);
	}
}

void UnloadUnitResources()
{
	for (int i = 0; i < unitMeshes.size(); i++)
		if (unitMeshes[i] != NULL)
			delete unitMeshes[i];

	unitMeshes.clear();
}

//////////////////////////////////////////////////////////////////////////////////
//								UNIT											//
//////////////////////////////////////////////////////////////////////////////////

UNIT::UNIT(int _type, int _team, INTPOINT mp, TERRAIN* _terrain, IDirect3DDevice9* Dev) : MAPOBJECT()
{
	m_type = _type;
	m_team = _team;
	m_mappos = mp;
	m_pTerrain = _terrain;
	m_pDevice = Dev;
	m_mapsize.Set(1, 1);
	m_time = 0.0f;
	m_animation = m_activeWP = 0;
	m_rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_scale = D3DXVECTOR3(0.2f, 0.2f, 0.2f);
	m_isBuilding = m_moving = false;
	m_movePrc = 0.0f;

	if (m_pTerrain != NULL)
		m_position = m_pTerrain->GetWorldPos(m_mappos);
	else m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if (m_type == 0)		//Farmer
	{
		m_hp = m_hpMax = 100;
		m_range = 1;
		m_damage = 5;
		m_sightRadius = 7;
		m_speed = 1.0f;
		m_name = "Farmer";
	}
	else if (m_type == 1)		//Soldier
	{
		m_hp = m_hpMax = 180;
		m_range = 1;
		m_damage = 12;
		m_sightRadius = 8;
		m_speed = 0.8f;
		m_name = "Soldier";
	}
	else if (m_type == 2)		//Magician
	{
		m_hp = m_hpMax = 100;
		m_range = 5;
		m_damage = 8;
		m_sightRadius = 10;
		m_speed = 1.1f;
		m_name = "Magician";
	}

	m_pAnimControl = unitMeshes[m_type]->GetAnimationControl();
	if (m_pAnimControl != NULL)
		m_pAnimControl->ResetTime();

	SetAnimation("Still");
}

UNIT::~UNIT()
{

}

void UNIT::Render()
{
	if (m_type < unitMeshes.size() && unitMeshes[m_type] != NULL)
	{
		SetAnimation(m_animation);

		unitMeshes[m_type]->SetPose(GetWorldMatrix(), m_pAnimControl, m_time);
		unitMeshes[m_type]->Render(NULL);
		m_time = 0.0f;
	}
}

void UNIT::Update(float deltaTime)
{
	//update unit animation time
	m_time += deltaTime * 0.8f * m_speed;

	//if the unit is moving...
	if (m_moving)
	{
		if (m_movePrc < 1.0f)m_movePrc += deltaTime * m_speed;
		if (m_movePrc > 1.0f)m_movePrc = 1.0f;

		//waypoint reached
		if (m_movePrc == 1.0f)
		{
			if (m_activeWP + 1 >= m_path.size())		//goal reached
			{
				m_moving = false;
				SetAnimation("Still");
			}
			else if (!CheckCollision(m_path[m_activeWP + 1])) //Next Waypoint
			{
				m_activeWP++;
				SetAnimation("Run");
				MoveUnit(m_path[m_activeWP]);
			}
		}

		//Interpolate position between m_lastWP and m_nextWP
		m_position = m_lastWP * (1.0f - m_movePrc) + m_nextWP * m_movePrc;
	}
}

bool UNIT::CheckCollision(INTPOINT mp)
{
	return false;		//No Collision
}

void UNIT::Goto(INTPOINT mp)
{
	if (m_pTerrain == NULL)return;

	//Clear old path
	m_path.clear();
	m_activeWP = 0;

	if (m_moving)		//If unit is currently moving
	{
		//Finish the active waypoint 
		m_path.push_back(m_mappos);
		std::vector<INTPOINT> tmpPath = m_pTerrain->GetPath(m_mappos, mp);

		//add new path
		for (int i = 0; i < tmpPath.size(); i++)
			m_path.push_back(tmpPath[i]);
	}
	else		//Create new path from scratch...
	{
		m_path = m_pTerrain->GetPath(m_mappos, mp);

		if (m_path.size() > 0)		//if a path was found
		{
			m_moving = true;

			//Check that the next tile is free
			if (!CheckCollision(m_path[m_activeWP]))
			{
				MoveUnit(m_path[m_activeWP]);
				SetAnimation("Run");
			}
		}
	}
}

void UNIT::MoveUnit(INTPOINT to)
{
	m_lastWP = m_pTerrain->GetWorldPos(m_mappos);
	m_rotation = GetDirection(m_mappos, to);

	m_mappos = to;		//New mappos
	m_movePrc = 0.0f;
	m_nextWP = m_pTerrain->GetWorldPos(m_mappos);
}

BBOX UNIT::GetBoundingBox()
{
	if (m_type == 0)			//Farmer
		return BBOX(m_position + D3DXVECTOR3(0.3f, 1.0f, 0.3f), m_position - D3DXVECTOR3(0.3f, 0.0f, 0.3f));
	else if (m_type == 1)	//Soldier
		return BBOX(m_position + D3DXVECTOR3(0.35f, 1.2f, 0.35f), m_position - D3DXVECTOR3(0.35f, 0.0f, 0.35f));
	else if (m_type == 2)	//Magician
		return BBOX(m_position + D3DXVECTOR3(0.3f, 1.1f, 0.3f), m_position - D3DXVECTOR3(0.3f, 0.0f, 0.3f));
}

D3DXMATRIX UNIT::GetWorldMatrix()
{
	D3DXMATRIX s, p, r;
	D3DXMatrixTranslation(&p, m_position.x, m_position.y, m_position.z);
	D3DXMatrixRotationYawPitchRoll(&r, m_rotation.y, m_rotation.x, m_rotation.z);
	D3DXMatrixScaling(&s, m_scale.y, m_scale.x, m_scale.z);
	return s * r * p;
}

D3DXVECTOR3 UNIT::GetDirection(INTPOINT p1, INTPOINT p2)
{
	int dx = p2.x - p1.x, dy = p2.y - p1.y;

	if (dx < 0 && dy > 0)	return D3DXVECTOR3(0.0f, D3DX_PI / 4, 0.0f);
	if (dx == 0 && dy > 0)	return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	if (dx > 0 && dy > 0)	return D3DXVECTOR3(0.0f, -D3DX_PI / 4, 0.0f);
	if (dx > 0 && dy == 0)	return D3DXVECTOR3(0.0f, -D3DX_PI / 2, 0.0f);
	if (dx > 0 && dy < 0)	return D3DXVECTOR3(0.0f, (-D3DX_PI / 4) * 3, 0.0f);
	if (dx == 0 && dy < 0)	return D3DXVECTOR3(0.0f, D3DX_PI, 0.0f);
	if (dx < 0 && dy < 0)	return D3DXVECTOR3(0.0f, (D3DX_PI / 4) * 3, 0.0f);
	if (dx < 0 && dy == 0)	return D3DXVECTOR3(0.0f, D3DX_PI / 2, 0.0f);

	return m_rotation;
}

void UNIT::SetAnimation(const char name[])
{
	ID3DXAnimationSet* anim = NULL;
	m_animation = 0;

	for (int i = 0; i < m_pAnimControl->GetMaxNumAnimationSets(); i++)
	{
		anim = NULL;
		m_pAnimControl->GetAnimationSet(i, &anim);

		if (anim != NULL)
		{
			if (strcmp(name, anim->GetName()) == 0)
			{
				m_pAnimControl->ResetTime();
				m_pAnimControl->SetTrackAnimationSet(0, anim);
				m_animation = i;
			}
			anim->Release();
		}
	}
}

void UNIT::SetAnimation(int index)
{
	ID3DXAnimationSet* anim = NULL;
	m_pAnimControl->GetAnimationSet(index, &anim);
	if (anim != NULL)m_pAnimControl->SetTrackAnimationSet(0, anim);
	anim->Release();
}