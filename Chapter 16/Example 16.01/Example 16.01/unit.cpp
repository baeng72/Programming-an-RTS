#include "unit.h"
#include "groupai.h"
#include "player.h"

std::vector<SKINNEDMESH*> unitMeshes;

void LoadUnitResources(IDirect3DDevice9* m_pDevice)
{
	std::vector<std::string> fnames;

	fnames.push_back("units/drone.x");
	fnames.push_back("units/soldier.x");
	fnames.push_back("units/magician.x");

	for (int i = 0; i < fnames.size(); i++)
	{
		SKINNEDMESH* newMesh = new SKINNEDMESH();
		newMesh->Load((char*)fnames[i].c_str(), m_pDevice);
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

UNIT::UNIT(int _type, int _team, INTPOINT mp, TERRAIN* _terrain, PLAYER* _player, IDirect3DDevice9* Dev) : MAPOBJECT()
{
	m_type = _type;
	m_team = _team;
	m_mappos = mp;
	m_pTerrain = _terrain;
	m_pDevice = Dev;
	m_mapsize.Set(0, 0);
	m_time = m_pauseTime = 0.0f;
	m_animation = m_activeWP = 0;
	m_rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_isBuilding = m_moving = false;
	m_movePrc = 0.0f;
	m_state = STATE_IDLE;
	m_attackTime = 0.0f;
	m_pPlayer = _player;
	m_mana = 0.0f;

	//Random sized units
	float y = (((rand() % 100) / 1000.0f) - 0.05f) * 0.333f;
	float xz = (((rand() % 100) / 1000.0f) - 0.05f) * 0.333f;
	m_scale = D3DXVECTOR3(0.2f + xz, 0.2f + y, 0.2f + xz);

	if (m_pTerrain != NULL)
	{
		m_position = m_pTerrain->GetWorldPos(m_mappos);
		MAPTILE* tile = m_pTerrain->GetTile(m_mappos);
		if (tile != NULL)tile->m_pMapObject = this;
	}
	else m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if (m_type == WORKER)
	{
		m_hp = m_hpMax = 100;
		m_range = 1;
		m_damage = 5;
		m_sightRadius = 7;
		m_speed = 1.0f;
		m_name = "Farmer";
	}
	else if (m_type == SOLDIER)
	{
		m_hp = m_hpMax = 180;
		m_range = 1;
		m_damage = 12;
		m_sightRadius = 8;
		m_speed = 0.8f;
		m_name = "Soldier";
	}
	else if (m_type == MAGICIAN)
	{
		m_hp = m_hpMax = 100;
		m_range = 4;
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
	if (m_pTerrain)m_pTerrain->m_updateSight = true;
}

void UNIT::Render()
{
	try
	{
		if (m_visible && m_type < unitMeshes.size() && unitMeshes[m_type] != NULL)
		{
			SetAnimation(m_animation);
			unitMeshes[m_type]->SetPose(GetWorldMatrix(), m_pAnimControl, m_time);
			unitMeshes[m_type]->Render(NULL);
			m_time = 0.0f;

			//Extract staff position
			if (m_type == MAGICIAN)
			{
				BONE* staff = unitMeshes[m_type]->FindBone("Staff");

				if (staff != NULL)
				{
					D3DXMATRIX mat = staff->CombinedTransformationMatrix;
					m_staffPos = D3DXVECTOR3(mat(3, 0), mat(3, 1), mat(3, 2));
				}
			}
		}
	}
	catch (...)
	{
		debug.Print("Error in UNIT::Render()");
	}
}

void UNIT::Update(float deltaTime)
{
	try
	{
		if (m_type == MAGICIAN && m_mana < 50)
			m_mana += deltaTime;

		//Pause the units...
		if (m_pauseTime > 0.0f && !m_dead)
		{
			m_pauseTime -= deltaTime;
			return;
		}

		m_staffPos = m_position + D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		//update unit animation time
		if (m_dead)
		{
			m_selected = false;
			m_state = STATE_DEAD;
			m_time += deltaTime * 0.3f;

			//Shrink sightradius when a unit has died
			int oldSr = m_sightRadius;
			if (m_sightRadius > 0.0f)m_sightRadius -= deltaTime;
			if (m_sightRadius < 0.0f)m_sightRadius = 0.0f;
			if (oldSr != (int)m_sightRadius)m_pTerrain->m_updateSight = true;

			float duration = unitMeshes[m_type]->GetAnimationDuration(ANIM_DIE) - 0.05f;
			if (m_pAnimControl->GetTime() > duration)
			{
				m_position.y -= deltaTime * 0.02f;
				m_time = 0.0f;
				m_pAnimControl->SetTrackPosition(0, duration);
			}
		}
		else if (m_state == STATE_ATTACK)
		{
			m_time += deltaTime * 0.5f;
			float attackDuration = unitMeshes[m_type]->GetAnimationDuration(ANIM_ATTACK);
			if (m_pAnimControl->GetTime() > attackDuration)
			{
				m_pAnimControl->SetTrackPosition(0, 0.0);
				m_time = 0.0f;
			}

			m_attackTime += deltaTime;
		}
		else m_time += deltaTime * 0.8f * m_speed;

		//Check that the unit is standing on a walkable tile
		MAPTILE* tile = m_pTerrain->GetTile(m_mappos);
		if (tile == NULL || !tile->m_walkable)
		{
			m_mappos = m_pTerrain->GetClosestFreeTile(m_mappos, m_mappos);
			MoveUnit(m_mappos);
			m_position = m_pTerrain->GetWorldPos(m_mappos);
			m_moving = false;
		}

		//if the unit is moving...
		if (m_moving && !m_dead)
		{
			if (m_movePrc < 1.0f)m_movePrc += deltaTime * m_speed;
			if (m_movePrc > 1.0f)m_movePrc = 1.0f;

			//Goal reached
			if (m_movePrc == 1.0f)
			{
				if (m_activeWP + 1 >= m_path.size())
				{
					m_moving = false;
					SetAnimation("Still");
					if (m_mappos != m_finalGoal)
						Goto(m_finalGoal, false, true, m_state);
				}
				else if (!CheckCollision(m_path[m_activeWP + 1]) && !UnitAI(true)) //Next Waypoint
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
	catch (...)
	{
		debug.Print("Error in UNIT::Update()");
	}
}

bool UNIT::CheckCollision(INTPOINT mp)
{
	MAPTILE* tile = m_pTerrain->GetTile(mp);
	if (tile == NULL || m_dead)return false;

	try
	{
		if (tile->m_pMapObject != NULL && tile->m_pMapObject != this)	//Collision with another unit
		{
			UNIT* otherUnit = (UNIT*)tile->m_pMapObject;

			//The other unit is moving
			if (otherUnit->m_moving && otherUnit->m_pauseTime <= 0.0f && m_speed <= otherUnit->m_speed)
			{
				//Pause the unit and wait for the other one to move 
				Pause((100 + rand() % 200) / 1000.0f);
				m_path.clear();
			}
			else	//Recalculate path
			{
				//Find next unoccupied walkable tile
				INTPOINT tempGoal = m_mappos;
				for (int i = m_activeWP + 1; i < m_path.size(); i++)
				{
					MAPTILE* tile = m_pTerrain->GetTile(m_path[i]);
					if (tile != NULL)
						if (tile->m_walkable && tile->m_pMapObject == NULL)
						{
							tempGoal = m_path[i];
							break;
						}
				}

				//No available tile found 
				if (tempGoal == m_mappos)
				{
					//Move to tile closest to the original goal
					INTPOINT newGoal = m_pTerrain->GetClosestFreeTile(m_finalGoal, m_mappos);

					if (newGoal == m_mappos || m_mappos.Distance(m_finalGoal) < 2.0f || !m_pTerrain->Within(newGoal))
					{
						m_moving = false;
						SetAnimation("Still");
						m_state = STATE_IDLE;
					}
					else Goto(newGoal, false, true, m_state);
				}
				else
				{
					//Move to tempGoal to avoid unit, then continue to m_finalGoal
					Goto(tempGoal, true, false, m_state);
				}
			}

			return true;	//A Collision happened
		}
	}
	catch (...)
	{
		debug.Print("Error in UNIT::CheckCollision()");
	}

	return false;		//No Collision
}

void UNIT::Goto(INTPOINT mp, bool considerUnits, bool _finalGoal, int newState)
{
	if (m_pTerrain == NULL || m_dead || !m_pTerrain->Within(mp))return;
	if (_finalGoal)m_finalGoal = mp;

	try
	{
		//Clear old path
		m_path.clear();
		m_activeWP = 0;
		m_state = newState;

		if (m_moving)		//If unit is currently moving
		{
			//Finish the active waypoint 
			m_path.push_back(m_mappos);
			std::vector<INTPOINT> tmpPath = m_pTerrain->GetPath(m_mappos, mp, considerUnits, this);

			//add new path
			for (int i = 0; i < tmpPath.size(); i++)
				m_path.push_back(tmpPath[i]);
		}
		else		//Create new path from scratch...
		{
			m_path = m_pTerrain->GetPath(m_mappos, mp, considerUnits, this);

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
			else Pause(0.1f);
		}
	}
	catch (...)
	{
		debug.Print("Error in UNIT::Goto()");
	}
}

void UNIT::MoveUnit(INTPOINT to)
{
	if (!m_pTerrain->Within(to))return;

	try
	{
		if (m_mappos.Distance(to) >= 2.0f)
		{
			m_state = STATE_IDLE;
			MoveUnit(m_mappos);
			m_moving = false;
			return;
		}

		m_lastWP = m_pTerrain->GetWorldPos(m_mappos);
		m_rotation = GetDirection(m_mappos, to);

		//Clear old MAPTILE unit pointer
		MAPTILE* tile = m_pTerrain->GetTile(m_mappos);
		if (tile != NULL)tile->m_pMapObject = NULL;

		m_mappos = to;	//New m_mappos
		m_movePrc = 0.0f;
		m_nextWP = m_pTerrain->GetWorldPos(m_mappos);

		//Set new MAPTILE unit pointer
		tile = m_pTerrain->GetTile(m_mappos);
		if (tile != NULL)tile->m_pMapObject = this;

		m_pTerrain->m_updateSight = true;
	}
	catch (...)
	{
		debug.Print("Error in UNIT::MoveUnit()");
	}
}

void UNIT::Pause(float time)
{
	SetAnimation("Still");
	m_pauseTime = time;
}

BBOX UNIT::GetBoundingBox()
{
	if (m_type == 0)		//Farmer
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
				m_pAnimControl->SetTrackAnimationSet(0, anim);
				m_pAnimControl->SetTrackPosition(0, 0.0f);
				m_pAnimControl->ResetTime();
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

bool UNIT::UnitAI(bool newMapTile)
{
	if (m_dead)return false;

	bool result = false;

	try
	{
		std::vector<MAPOBJECT*> enemies = GetTargetsWithinRange(m_sightRadius);

		//Report enemy sightings
		if (m_pGroup != NULL && !enemies.empty())
			m_pGroup->EnemiesSpotted(enemies);

		switch (m_state)
		{
		case STATE_IDLE:
		{
			m_animation = ANIM_STILL;

			if (enemies.size() > 0)
			{
				m_pTarget = BestTargetToAttack(enemies);
				m_state = STATE_ATTACK;
				result = true;
			}

			Heal();

			break;
		}
		case STATE_MOVING:
		{
			m_animation = ANIM_RUN;

			if (!m_moving)
			{
				m_state = STATE_IDLE;
				m_animation = ANIM_STILL;
			}

			break;
		}
		case STATE_DEAD:
		{
			//Stay dead
			m_dead = true;
			break;
		}
		case STATE_SEARCH:
		{
			if (enemies.size() > 1)
				m_pTarget = BestTargetToAttack(enemies);

			if (m_pTarget == NULL || m_pTarget->m_dead)
			{
				m_state = STATE_IDLE;
				break;
			}

			//Attack if target is within range
			if (newMapTile || !m_moving)
			{
				if (m_mappos.inRect(m_pTarget->GetMapRect(m_range)))
				{
					m_state = STATE_ATTACK;
					SetAnimation("Attack");
					m_moving = false;
					m_attackTime = 0.0f;
					m_time = unitMeshes[m_type]->GetAnimationDuration(ANIM_ATTACK);
					m_rotation = GetDirection(m_mappos, m_pTarget->m_mappos);
					result = true;
				}
				else
				{
					INTPOINT attackPos = m_pTarget->GetAttackPos(m_mappos);
					if (!m_pTerrain->Within(attackPos))
					{
						m_state = STATE_IDLE;
						break;
					}

					//Update path
					if (!m_moving || attackPos != m_finalGoal)
					{
						bool considerUnit = m_mappos.Distance(m_pTarget->m_mappos) <= 5;
						Goto(attackPos, considerUnit, true, m_state);
					}
				}
			}

			break;
		}
		case STATE_ATTACK:
		{
			if (m_pTarget == NULL || m_pTarget->m_dead) { m_state = STATE_IDLE; return false; }

			//Heal other units
			Heal();

			if (m_mappos.inRect(m_pTarget->GetMapRect(m_range)))
			{
				//Attack
				if (m_attackTime > 3.0f)
				{
					m_attackTime = m_time = 0.0f;
					m_animation = ANIM_ATTACK;
					m_rotation = GetDirection(m_mappos, m_pTarget->m_mappos);
					m_pAnimControl->ResetTime();

					//Magician Fireball effect
					if (m_type == MAGICIAN)
						effects.push_back(new EFFECT_FIREBALL(m_pDevice, &m_staffPos, m_pTarget, this));
					else m_pTarget->Damage(m_damage, this);
				}
			}
			else m_state = STATE_SEARCH;

			break;
		}
		case STATE_RETREAT:
		{
			if (!m_moving)
				m_state = STATE_IDLE;

			break;
		}
		case STATE_GOTO_BUILD:
		{
			if (!m_moving)Goto(m_buildingPosition, false, true, STATE_GOTO_BUILD);

			BUILDING* temp = new BUILDING(m_buildingToPlace, m_team, true, m_buildingPosition, m_pTerrain, m_pPlayer, false, m_pDevice);

			if (m_mappos.inRect(temp->GetMapRect(0)))
			{
				if (m_pPlayer->money < GetCost(m_buildingToPlace, true))
				{
					m_state = STATE_IDLE;
				}
				else
				{
					m_selected = false;
					m_pPlayer->RemoveMapObject(this);
					BUILDING* build = (BUILDING*)m_pPlayer->AddMapObject(m_buildingToPlace, m_buildingPosition, true, false);
					build->m_pTarget = this;
					m_pPlayer->money -= GetCost(m_buildingToPlace, true);
					m_state = STATE_BUILD;
				}
			}

			delete temp;

			break;
		}
		case STATE_BUILD:
		{
			m_state = STATE_IDLE;
			break;
		}
		}
	}
	catch (...)
	{
		debug.Print("Error in UNIT::UnitAI()");
	}

	return result;
}

void UNIT::Attack(MAPOBJECT* _target)
{
	if (m_dead || _target == NULL)return;

	m_pTarget = _target;
	m_state = STATE_SEARCH;
	UnitAI(false);
}

void UNIT::ConstructBuilding(int buildToPlace, INTPOINT pos)
{
	if (m_type != WORKER)return;

	Goto(pos, false, true, STATE_GOTO_BUILD);
	m_buildingToPlace = buildToPlace;
	m_buildingPosition = pos;
	UnitAI(false);
}

bool UNIT::isDead()
{
	if (!m_dead || m_pTerrain == NULL)return false;

	D3DXVECTOR3 worldPos = m_pTerrain->GetWorldPos(m_mappos);
	return m_dead && (worldPos.y - m_position.y) > 3.0f;
}

void UNIT::Damage(int dmg, MAPOBJECT* attacker)
{
	if (m_dead)return;

	try
	{
		m_hp -= dmg;

		if (m_hp <= 0)
		{
			m_hp = 0;
			m_dead = true;

			UNIT* attackUnit = (UNIT*)attacker;
			if (attackUnit != NULL && attackUnit->m_pPlayer != NULL)
				attackUnit->m_pPlayer->m_numKills++;

			//Play m_dead animation
			SetAnimation("Die");
			m_time = 0.0f;
			m_pTerrain->m_updateSight = true;

			//Reset mapTile->m_pMapObject to NULL
			if (m_pTerrain != NULL)
			{
				MAPTILE* tile = m_pTerrain->GetTile(m_mappos);
				if (tile != NULL)tile->m_pMapObject = NULL;
			}

			//Leave group
			if (m_pGroup != NULL)
			{
				m_pGroup->RemoveMember(this);
				m_pGroup = NULL;
			}
		}
	}
	catch (...)
	{
		debug.Print("Error in UNIT::Damage()");
	}
}

void UNIT::Heal()
{
	//Heal Spell
	if (m_type == MAGICIAN && m_mana > 40.0f)
	{
		RECT r = GetMapRect(m_sightRadius - 1);
		UNIT* bestUnit = NULL;
		float healthPrc = 0.6f;

		//Find the friendly unit with the lowest health
		for (int y = r.top; y <= r.bottom; y++)
			for (int x = r.left; x <= r.right; x++)
			{
				MAPTILE* tile = m_pTerrain->GetTile(x, y);

				if (tile != NULL && tile->m_pMapObject != NULL && !tile->m_pMapObject->m_dead &&
					tile->m_pMapObject->m_team == m_team && !tile->m_pMapObject->m_isBuilding &&
					tile->m_pMapObject->m_hp < tile->m_pMapObject->m_hpMax)
				{
					float h = tile->m_pMapObject->m_hp / (float)tile->m_pMapObject->m_hpMax;

					if (h < healthPrc)
					{
						healthPrc = h;
						bestUnit = (UNIT*)tile->m_pMapObject;
					}
				}
			}

		//Cast Heal Spell
		if (bestUnit != NULL)
		{
			bestUnit->m_hp += bestUnit->m_hpMax * 0.5f;
			if (bestUnit->m_hp > bestUnit->m_hpMax)
				bestUnit->m_hp = bestUnit->m_hpMax;

			effects.push_back(new EFFECT_SPELL(m_pDevice, bestUnit->m_position + D3DXVECTOR3(0.0f, 0.1f, 0.0f)));
			Pause(3.0f);
			bestUnit->Pause(5.0f);
			bestUnit->m_animation = ANIM_STILL;
			m_mana -= 40.0f;
		}
	}
}