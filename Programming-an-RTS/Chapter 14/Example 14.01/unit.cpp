#include "unit.h"
#include "GroupAI.h"
#include "player.h"
#include <iostream>
std::vector<std::unique_ptr<SKINNEDMESH>> unitMeshes;

void LoadUnitResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager) {
	std::vector<std::string> fnames = {
		"units/drone.x",
		"units/soldier.x",
		"units/magician.x"
	};
	unitMeshes.resize(fnames.size());
	for (int i = 0; i < fnames.size(); i++) {
		auto& fname = fnames[i];
		std::string path = "../../../../Resources/Chapter 11/Example 11.01/" + fname;

		unitMeshes[i] = std::make_unique<SKINNEDMESH>();
		unitMeshes[i]->Load(pdevice, shaderManager, path.c_str());


	}
}

void UnloadUnitResources() {
	unitMeshes.clear();
}



// UINT

UNIT::UNIT(Renderer::RenderDevice*pdevice,int type, int team, INTPOINT mp, TERRAIN* terrain,PLAYER*pplayer) {
	_pdevice = pdevice;
	_pPlayer = pplayer;
	_type = type;
	_team = team;
	_mappos = mp;
	_pTerrain = terrain;

	_mapsize.Set(1, 1);
	_time = _pauseTime = 0.f;
	_animation = _activeWP = 0;
	_rotation = vec3(0.f);
	_scale = vec3(0.2f);
	_isBuilding = _moving = false;
	_movePrc = 0.f;
	_state = STATE_IDLE;
	_attackTime = 0.f;
	_mana = 0.f;
	

	if (terrain) {
		_position = _pTerrain->GetWorldPos(_mappos);
		MAPTILE* tile = _pTerrain->GetTile(_mappos);
		if (tile)
			tile->_pMapObject = this;
	}
	else
		_position = vec3(0.f);

	if (_type == WORKER) {
		//farmer
		_hp = _hpMax = 100;
		_range = 1;
		_damage = 5;
		_sightRadius = 7;
		_speed = 1.f;
		_name = "Farmer";
	}
	else if (_type == SOLDIER) {
		//Soldier
		_hp = _hpMax = 180;
		_range = 1;
		_damage = 12;
		_sightRadius = 8;
		_speed = 0.8f;
		_name = "Soldier";
	}
	else if (_type == 2) {
		//Magician
		_hp = _hpMax = 100;
		_range = 5;
		_damage = 8;
		_sightRadius = 10;
		_speed = 1.1f;
		_name = "Magician";
	}

	unitMeshes[_type]->Update();
	_pcontroller = unitMeshes[_type]->GetAnimationController();
	SetAnimation("Still");
}

UNIT::~UNIT() {
	if (_pTerrain) {
		/*MAPTILE* tile = _pTerrain->GetTile(_mappos);
		if (tile)
			tile->_pMapObject = nullptr;*/
		_pTerrain->_updateSight = true;
	}
}

void UNIT::Render(Renderer::Shader* pshader) {

	if (_visible && _type < unitMeshes.size() && unitMeshes[_type]) {
		{
			{

				SetAnimation(_animation);
			}
			{
				unitMeshes[_type]->SetPose(_pcontroller,_time);
				_time = 0.f;
			}
		}
		mat4 worldxform = unitMeshes[_type]->GetWorldXForm() * GetWorldMatrix();
		vec4 color = vec4(1.f, 0.f, 0.f, 1.f);
		pshader->SetUniform("model", &worldxform);
		pshader->SetUniform("color", &color);
		
		unitMeshes[_type]->Render(_pcontroller,pshader);

		//extract staff position
		if (_type == MAGICIAN) {
			mat4 bone;
			_pcontroller->GetBonePoseXForm(_pcontroller->GetBoneIndex("Bone17"), bone);
			_staffPos = vec3(bone[3]) + vec3(1.f, -2.f, 11.f);//hack, pure and simple. We don't load bones that have no animation channels, i.e. the bone 'Staff', so need to align with a nearby bone that exists
		}

	}
}

void UNIT::Update(float deltaTime) {
	//update unit animation time

	if (_type == MAGICIAN && _mana < 50)
		_mana += deltaTime;

	if (_pauseTime > 0.f && !_dead) {
		_pauseTime -= deltaTime;
		return;
	}

	_staffPos = _position + vec3(0.f, 1.f, 0.f);

	if (_dead) {
		_selected = false;
		_state = STATE_DEAD;
		_time += deltaTime * 0.3f;
		//shrink sight radius when a unit has died
		int oldSrc = _sightRadius;
		_sightRadius = std::max(0.f, _sightRadius - deltaTime);

		if (oldSrc != (int)_sightRadius)
			_pTerrain->_updateSight = true;

		float duration = unitMeshes[_type]->GetAnimationDuration(ANIM_DIE) - 0.5f;
		if (unitMeshes[_type]->GetAnimationTime() > duration) {
			_position.y -= deltaTime * 0.02f;
			_time = 0.f;
			_pcontroller->SetTime(duration);
		}
	}
	else if (_state == STATE_ATTACK) {
		_time += deltaTime * 0.5f;
		float attackDuration = unitMeshes[_type]->GetAnimationDuration(ANIM_ATTACK);
		if (_pcontroller->GetTime() > attackDuration) {
			_pcontroller->SetTime(0.f);
			_time = 0.f;
		}
		_attackTime += deltaTime;
	}
	else
		_time += deltaTime * 0.8f * _speed * 100;

	//check that the unit is standing on a walkable tile
	MAPTILE* ptile = _pTerrain->GetTile(_mappos);
	if (!ptile || !ptile->_walkable) {
		_mappos = _pTerrain->GetClosestFreeTile(_mappos, _mappos);
		MoveUnit(_mappos);
		_position = _pTerrain->GetWorldPos(_mappos);
		_moving = false;
	}

	//if the unit is moving
	if (_moving) {
		if (_movePrc < 1.f)
			_movePrc += deltaTime * _speed;
		if (_movePrc > 1.f)
			_movePrc = 1.f;

		//waypoint reached
		if (_movePrc == 1.f) {
			if (_activeWP + 1 >= _path.size()) {
				//goal reaced
				_moving = false;
				SetAnimation("Still");
				if (_mappos != _finalGoal)
					Goto(_finalGoal, false, true, _state);
			}
			else if (!CheckCollision(_path[_activeWP]) && !UnitAI(true)) {//Next waypoint
				_activeWP++;
				SetAnimation("Run");
				MoveUnit(_path[_activeWP]);

			}
		}
		//Interpolate position between _lastWP and _nextWP
		_position = _lastWP * (1.f - _movePrc) + _nextWP * _movePrc;
	}
}


bool UNIT::CheckCollision(INTPOINT mp) {
	MAPTILE* ptile = _pTerrain->GetTile(mp);
	if (!ptile || _dead)
		return false;

	if (ptile->_pMapObject && ptile->_pMapObject != this) {//collision with another unit
		UNIT* potherunit = (UNIT*)ptile->_pMapObject;

		//the other unit is moving
		if (potherunit->_moving && potherunit->_pauseTime <= 0.f && _speed <= potherunit->_speed) {
			//Pause unit and wait for the other one to move
			Pause((100 + rand() % 200) / 1000.f);
			_path.clear();
		}
		else {//recalculate path
			//Find next unoccuplied walkable tile
			INTPOINT tempGoal = _mappos;
			for (int i = _activeWP + 1; i < _path.size(); i++) {
				MAPTILE* ptile = _pTerrain->GetTile(_path[i]);
				if (ptile) {
					if (ptile->_walkable && !ptile->_pMapObject) {
						tempGoal = _path[i];
						break;
					}
				}
			}
			//No available tile found
			if (tempGoal == _mappos) {
				//Move to tile closest to the original goal
				INTPOINT newGoal = _pTerrain->GetClosestFreeTile(_finalGoal, _mappos);

				if (newGoal == _mappos || _mappos.Distance(_finalGoal) < 2.f || !_pTerrain->Within(newGoal)) {
					_moving = false;
					SetAnimation("Still");
					_state = STATE_IDLE;

				}
				else
					Goto(tempGoal, true, false, _state);
			}
		}
		return true;//a collision happend
	}
	return false;//no collision
}

void UNIT::Goto(INTPOINT mp, bool considerUnits, bool finalGoal,int newState) {
	if (!_pTerrain) {
		return;
	}
	if (finalGoal)
		_finalGoal = mp;
	//clear old path
	_path.clear();
	_activeWP = 0;
	_state = newState;

	if (_moving) {	//if unit is currently moving
		//Finish the active waypoint
		_path.push_back(_mappos);
		std::vector<INTPOINT> tmpPath = _pTerrain->GetPath(_mappos, mp, considerUnits,this);
		//add new path
		_path.insert(_path.end(), tmpPath.begin(), tmpPath.end());
	}
	else {
		_path = _pTerrain->GetPath(_mappos, mp, considerUnits,this);

		if (_path.size()) {	//if a path was found
			_moving = true;
			//check that the next tile if free

			if (!CheckCollision(_path[_activeWP])) {
				MoveUnit(_path[_activeWP]);
				SetAnimation("Run");
			}
		}
		else
			Pause(0.1f);
	}
}

void UNIT::MoveUnit(INTPOINT to) {
	if (!_pTerrain->Within(to))
		return;
	if (_mappos.Distance(to) > 2.f) {
		_state = STATE_IDLE;
		MoveUnit(_mappos);
		_moving = false;
		return;
	}
	_lastWP = _pTerrain->GetWorldPos(_mappos);
	_rotation = GetDirection(_mappos, to);

	//clear old MAPTILE unit pointer
	MAPTILE* ptile = _pTerrain->GetTile(_mappos);
	if (ptile) {
		ptile->_pMapObject = nullptr;
	}

	_mappos = to;
	_movePrc = 0.f;
	_nextWP = _pTerrain->GetWorldPos(_mappos);
	_pTerrain->_updateSight = true;

	//Set new MAPTILE unit pointer
	ptile = _pTerrain->GetTile(_mappos);
	if (ptile)
		ptile->_pMapObject = this;

	_pTerrain->_updateSight = true;
}

void UNIT::Pause(float time) {
	SetAnimation("Still");
	_pauseTime = time;
}

BBOX UNIT::GetBoundingBox() {
	if (_type == 0) {
		//Farmer
		return BBOX(_position + vec3(0.3f, 1.f, 0.3f), _position - vec3(0.3f, 0.f, 0.3f));
	}
	else if (_type == 1) {
		//Soldier
		return BBOX(_position + vec3(0.35f, 1.2f, 0.35f), _position - vec3(0.35f, 0.f, 0.35f));
	}
	else if (_type == 2) {
		return BBOX(_position + vec3(0.3f, 1.1f, 0.3f), _position - vec3(0.3f, 0.f, 0.3f));
	}
	return BBOX();
}

mat4 UNIT::GetWorldMatrix() {
	mat4 s, p, r;
	mat4 id = mat4(1.f);
	p = translate(id, _position);
	quat q = quat(_rotation);
	r = mat4(q);// rotate(rotate(rotate(id, _rotation.x, vec3(1.f, 0.f, 0.f)), _rotation.y, vec3(0.f, 1.f, 0.f)), _rotation.z, vec3(0.f, 0.f, 1.f));
	s = scale(id, _scale);
	return p * r * s;
}

vec3 UNIT::GetDirection(INTPOINT p1, INTPOINT p2)
{
	int dx = p2.x - p1.x, dy = p2.y - p1.y;

	if (dx < 0 && dy > 0)	return vec3(0.0f, glm::pi<float>() / 4, 0.0f);
	if (dx == 0 && dy > 0)	return vec3(0.0f, 0.0f, 0.0f);
	if (dx > 0 && dy > 0)	return vec3(0.0f, -glm::pi<float>() / 4, 0.0f);
	if (dx > 0 && dy == 0)	return vec3(0.0f, -glm::pi<float>() / 2, 0.0f);
	if (dx > 0 && dy < 0)	return vec3(0.0f, (-glm::pi<float>() / 4) * 3, 0.0f);
	if (dx == 0 && dy < 0)	return vec3(0.0f, glm::pi<float>(), 0.0f);
	if (dx < 0 && dy < 0)	return vec3(0.0f, (glm::pi<float>() / 4) * 3, 0.0f);
	if (dx < 0 && dy == 0)	return vec3(0.0f, glm::pi<float>() / 2, 0.0f);

	return _rotation;
}

void UNIT::SetAnimation(const char* name)
{

	_animation = 0;
	auto& animNames = unitMeshes[_type]->GetAnimations();
	for (int i = 0; i < animNames.size(); i++)
	{
		if (animNames[i] == name)
		{
			_animation = i;
			
			break;
		}
	}
	_pcontroller->SetAnimation(_animation, false);
}

void UNIT::SetAnimation(int i) {
	if (_dead)
		return;
	_animation = i;
	_pcontroller->SetAnimation(i,false);	
}

Renderer::Shader* UNIT::GetShader() {
	return unitMeshes[_type]->GetShader();
}

bool UNIT::UnitAI(bool newMapTile) {
	if (_dead)
		return false;
	bool result = false;

	std::vector<MAPOBJECT*> enemies = GetTargetsWithinRange(_sightRadius);

	//report enemy sightings
	if (_pGroup && !enemies.empty()) {
		_pGroup->EnemiesSpotted(enemies);
	}

	switch (_state) {
	case STATE_IDLE: {
		_animation = ANIM_STILL;

		if (enemies.size() > 0) {
			_pTarget = BestTargetToAttack(enemies);
			_state = STATE_ATTACK;
			result = true;
		}
		Heal();

		break;
	}
	case STATE_MOVING: {
		_animation = ANIM_RUN;

		if (!_moving) {
			_state = STATE_IDLE;
			_animation = ANIM_STILL;
		}
		break;
	}
	case STATE_DEAD:
	{
		_dead = true;
		break;
	}
	case STATE_SEARCH:
	{
		if (enemies.size() > 1)
			_pTarget = BestTargetToAttack(enemies);

		if (!_pTarget || _pTarget->_dead) {
			_state = STATE_IDLE;
			break;
		}

		//Attack if target is within range
		if (newMapTile || !_moving) {
			if (_mappos.inRect(_pTarget->GetMapRect(_range))) {
				_state = STATE_ATTACK;
				SetAnimation("Attack");
				_moving = false;
				_attackTime = 0.f;
				_time = unitMeshes[_type]->GetAnimationDuration(ANIM_ATTACK);
				_rotation = GetDirection(_mappos, _pTarget->_mappos);
				result = true;
			}
			else
			{
				INTPOINT attackPos = _pTarget->GetAttackPos(_mappos);
				if (!_pTerrain->Within(attackPos)) {
					_state = STATE_IDLE;
					break;
				}

				//update path
				if (!_moving || attackPos != _finalGoal) {
					bool considerUnit = _mappos.Distance(_pTarget->_mappos) <= 5;
					Goto(attackPos, considerUnit, true, _state);
				}
			}
		}
		break;
	}
	case STATE_ATTACK:
	{
		if (!_pTarget || _pTarget->_dead) {
			_state = STATE_IDLE;
			return false;
		}

		Heal();//heal other units

		if (_mappos.inRect(_pTarget->GetMapRect(_range))) {
			//Attack
			if (_attackTime > 3.f) {
				_attackTime = 0.f;
				_animation = ANIM_ATTACK;
				_rotation = GetDirection(_mappos, _pTarget->_mappos);
				_pcontroller->SetTime(0.f);

				//magician fireball effect
				if (_type == MAGICIAN)
					effects.push_back(new EFFECT_FIREBALL(_pdevice, _staffPos, _pTarget, this));
				else
					_pTarget->Damage(_damage, this);
			}
		}
		else
			_state = STATE_SEARCH;
		break;
	}
	case STATE_RETREAT:
	{
		if (!_moving)
			_state = STATE_IDLE;
		break;
	}
	case STATE_GOTO_BUILD:
	{
		if (!_moving)
			Goto(_buildingPosition, false, true, STATE_GOTO_BUILD);

		BUILDING* ptemp = new BUILDING(_pdevice,_buildingToPlace, _team, true, _buildingPosition, _pTerrain, _pPlayer, false);

		if (_mappos.inRect(ptemp->GetMapRect(0))) {
			if (_pPlayer->money < GetCost(_buildingToPlace, true)) {
				_state = STATE_IDLE;
			}
			else
			{
				_selected = false;
				_pPlayer->RemoveMapObject(this);
				BUILDING* pbuild = (BUILDING*)_pPlayer->AddMapObject(_buildingToPlace, _buildingPosition, true, false);
				pbuild->_pTarget = this;
				_pPlayer->money -= GetCost(_buildingToPlace, true);
				_state = STATE_BUILD;
			}
		}
		delete ptemp;
		break;
	}

	case STATE_BUILD: {
		_state = STATE_IDLE;
		break;
	}
	}
	return result;
}

void UNIT::Attack(MAPOBJECT* ptarget) {
	if (_dead || ptarget)
		return;

	_pTarget = ptarget;
	_state = STATE_SEARCH;
	UnitAI(false);
}

void UNIT::ConstructBuilding(int buildToPlace, INTPOINT pos) {
	if(_type != WORKER)
		return;

	Goto(pos, false, true, STATE_GOTO_BUILD);
	_buildingToPlace = buildToPlace;
	_buildingPosition = pos;
	UnitAI(false);
}

bool UNIT::isDead() {
	if (!_dead || _pTerrain)
		return false;

	vec3 worldPos = _pTerrain->GetWorldPos(_mappos);
	return _dead && (worldPos.y - _position.y) > 3.f;
}

void UNIT::Damage(int dmg, MAPOBJECT* pattacker) {
	if (_dead)
		return;

	_hp -= dmg;

	if (_hp <= 0) {
		_hp = 0;
		_dead = true;

		UNIT* pAttackUnit = (UNIT*)pattacker;
		if (pAttackUnit && pAttackUnit->_pPlayer)
			pAttackUnit->_pPlayer->_numKills++;

		//Play _dead _animation
		SetAnimation("Die");
		_time = 0.f;
		_pTerrain->_updateSight = true;

		//Reset mapTile->mapObject to null
		if (_pTerrain) {
			MAPTILE* ptile = _pTerrain->GetTile(_mappos);
			if (ptile)
				ptile->_pMapObject = nullptr;
		}

		//Leave group
		if (_pGroup) {
			_pGroup->RemoveMember(this);
			_pGroup = nullptr;
		}
	}
}

void UNIT::Heal() {
	//Heal Spell
	if (_type == MAGICIAN && _mana > 30.f) {
		Rect r = GetMapRect(_sightRadius - 1);
		UNIT* bestUnit = nullptr;
		float healthPrc = 0.6f;

		//Find the friendly unit with the lowest health
		for (int y = r.top; y <= r.bottom; y++) {
			for (int x = r.left; x <= r.right; x++) {
				MAPTILE* ptile = _pTerrain->GetTile(x, y);

				if (ptile && 
					ptile->_pMapObject && 
					!ptile->_pMapObject->_dead &&
					ptile->_pMapObject->_team == _team && 
					!ptile->_pMapObject->_isBuilding &&
					ptile->_pMapObject->_hp < ptile->_pMapObject->_hpMax) {
					float h = ptile->_pMapObject->_hp / (float)ptile->_pMapObject->_hpMax;

					if (h < healthPrc) {
						healthPrc = h;
						bestUnit = (UNIT*)ptile->_pMapObject;
					}
				}
			}
		}
		//Cast Heal Spell
		if (bestUnit) {
			bestUnit->_hp += bestUnit->_hpMax * 0.5f;
			if (bestUnit->_hp > bestUnit->_hpMax)
				bestUnit->_hp = bestUnit->_hpMax;

			effects.push_back(new EFFECT_SPELL(_pdevice, bestUnit->_position + vec3(0.f, 1.f, 0.f)));
			Pause(3.f);
			bestUnit->Pause(5.f);
			bestUnit->_animation = ANIM_STILL;
			_mana -= 30.f;
		}
	}
}