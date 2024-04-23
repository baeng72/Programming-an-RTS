#include "building.h"
#include "GroupAI.h"
#include "player.h"

std::vector< std::unique_ptr<MESH>> buildingMeshes;

void LoadBuildingResources(Renderer::RenderDevice* pdevice) {
	std::vector<std::string> fnames = {
		"townhall_b.x",
		"townhall.x",
		"barracks_b.x",
		"barracks.x",
		"tower_b.x",
		"tower.x"
	};
	for (int i = 0; i < fnames.size(); i++) {
		auto& fname = fnames[i];
		std::string path = Core::ResourcePath::GetMeshPath(fname.c_str());
		buildingMeshes.push_back(std::make_unique<MESH>(pdevice, path.c_str()));
	}
}

void UnloadBuildingResources() {
	buildingMeshes.clear();
}

bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* pterrain) {
	if (pterrain == nullptr)
		return false;

	BUILDING b(nullptr,buildType, 0,true, mp, nullptr,nullptr, false);
	Rect r = b.GetMapRect(2);

	for (int y = r.top; y <= r.bottom; y++) {
		for (int x = r.left; x <= r.right; x++) {
			//Building must be within map borders
			if (!pterrain->Within(INTPOINT(x, y)))
				return false;
			MAPTILE* ptile = pterrain->GetTile(x, y);
			if (ptile == nullptr)
				return false;

			//The terrain must be level and walkable
			if (ptile->_height != 0.f || !ptile->_walkable)
				return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////
/// BUILDING
/////////////////////////////////////////////////////

BUILDING::BUILDING(Renderer::RenderDevice*pdevice,int type, int team, bool finished, INTPOINT mp, TERRAIN* terrain,PLAYER*player, bool affectTerrain)
{
	_pdevice = pdevice;
	
	_type = type;
	_team = team;
	_mappos = mp;
	_pTerrain = terrain;
	_affectTerrain = affectTerrain;
	
	_range = _damage = 0;
	_isBuilding = true;
	_deathCountDown = 0.f;
	_pPlayer = player;
	_pTarget = nullptr;
	_pTrainingEffect = nullptr;
	_training = false;
	_meshInstance.SetMesh(buildingMeshes[_type].get());

	if (finished) {
		_buildProgress = 1.f;
		_meshInstance.SetMesh(buildingMeshes[_type * 2 + 1].get());
	}
	else {
		_buildProgress = 0.f;
		_meshInstance.SetMesh(buildingMeshes[_type * 2].get());
	}

	if (_type == TOWNHALL) {
		//Townhall
		_hp = _hpMax = 600;
		_sightRadius = 12;
		_name = "Townhall";
		_mapsize.Set(4, 2);
		_meshInstance.SetScale(vec3(0.13f));
		_buildSpeed = 0.05f;
	}
	else if (_type == BARRACKS) {
		//Barracks
		_hp = _hpMax = 450;
		_sightRadius = 13;
		_name = "Barracks";
		_mapsize.Set(2, 4);
		_meshInstance.SetScale(vec3(0.15f));
		_buildSpeed = 0.07f;
	}
	else if (_type == 2) {
		//Tower
		_hp = _hpMax = 750;
		_sightRadius = 20;
		_name = "Tower";
		_mapsize.Set(2, 2);
		_meshInstance.SetScale(vec3(0.13f));
		_buildSpeed = 0.04f;
	}

	if (_pTerrain)
		_position = _pTerrain->GetWorldPos(_mappos) + vec3(_mapsize.x / 2.f, 0.f, -_mapsize.y / 2.f);
	else
		_position = vec3(0.f);

	_meshInstance.SetPosition(_position);

	_BBox = _meshInstance.GetBoundingBox();
	_BBox.max -= vec3(0.2f);
	_BBox.min += vec3(0.2f);

	//Update the tiles of the terrain
	if (_pTerrain && _affectTerrain) {
		Rect mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++) {
			for (int x = mr.left; x <= mr.right; x++) {
				MAPTILE* ptile = _pTerrain->GetTile(x, y);
				if (ptile) {
					ptile->_walkable = false;
					ptile->_pMapObject = nullptr;
				}

			}
		}
		_pTerrain->_updateSight = true;
		_pTerrain->UpdatePathfinding(&GetMapRect(1));
		_pTrainingEffect = new EFFECT_TRAINING(_pdevice);
	}

	//set fire positions and scales
	if (_type == TOWNHALL) {
		_firePos[0] = _position + vec3(0.f, 3.f, 1.f);
		_firePos[1] = _position + vec3(-2.f, 1.2f, -0.4f);
		_firePos[2] = _position + vec3(2.f, 0.f, -1.f);
		_fireScale[0] = vec3(3.f, 4.f, 3.f);
		_fireScale[1] = vec3(3.f, 8.f, 3.f);
		_fireScale[2] = vec3(3.f, 8.f, 3.f);
	}
	else if (type == BARRACKS) {
		_firePos[0] = _position + vec3(-0.85f, 2.5f, 0.7f);
		_firePos[1] = _position + vec3(0.5f, 0.f, 0.f);
		_firePos[2] = _position + vec3(0.f, 0.f, -2.f);
		_fireScale[0] = vec3(3.f, 3.f, 3.5f);
		_fireScale[1] = vec3(2.f, 2.f, 3.5f);
		_fireScale[2] = vec3(3.f, 3.f, 4.f);
	}
	else if (type == TOWER) {
		_firePos[0] = _position + vec3(0.f, 0.f, 0.85f);
		_firePos[1] = _position + vec3(-0.6f, 3.f, -0.4f);
		_firePos[2] = _position + vec3(0.f, 5.f, 0.f);
		_fireScale[0] = vec3(1.5f, 1.5f, 3.f);
		_fireScale[1] = vec3(1.5f, 1.5f, 2.f);
		_fireScale[2] = vec3(3.f, 3.f, 4.f);
	}

}

BUILDING::~BUILDING()
{
	for (int i = 0; i < _fires.size(); i++) {
		if (_fires[i])
			delete _fires[i];	
	}
	_fires.clear();

	if (_pTrainingEffect)
		delete _pTrainingEffect;

}

void BUILDING::Render(Renderer::Shader* pshader)
{
	mat4 matWorld = _meshInstance.GetWorldMatrix();
	mat4 xform = _meshInstance.GetMeshXForm();
	mat4 worldxform = matWorld * xform;
	vec4 teamColor = vec4(1.f, 0.f, 0.f, 1.f);
	vec4 color = vec4(1.f);
	pshader->SetUniform("model", &worldxform);
	pshader->SetUniform("teamColor", &teamColor);
	pshader->SetUniform("color", &color);
	if (_visible) {
		if (_buildProgress < 1.f)
			_meshInstance.Render(pshader, _buildProgress);
	}else
		_meshInstance.Render(pshader);
}

void BUILDING::RenderFires(mat4&matProj,mat4&matView) {
	for (int i = 0; i < _fires.size(); i++) {
		if (_fires[i])
			_fires[i]->Render(matProj, matView);
	}
}

void BUILDING::Update(float deltaTime)
{
	//Train units, upgrade things etc here
	if (_dead) {
		_deathCountDown += deltaTime;
		_position.y -= deltaTime;
		_meshInstance._pos = _position;

		//shrink sightradius when a unit has died
		int oldSr = _sightRadius;
		if (_sightRadius > 0.f)
			_sightRadius -= deltaTime * 2.f;
		if (_sightRadius < 0.f)
			_sightRadius = 0.f;
		if (oldSr != (int)_sightRadius)
			_pTerrain->_updateSight = true;
	}
	else {
		//Building building
		if (_buildProgress < 1.f) {
			_buildProgress += deltaTime * _buildSpeed;

			if (_buildProgress >= 1.f) {
				_buildProgress = 1.f;
				_meshInstance.SetMesh(buildingMeshes[_type * 2 + 1].get());

				//Deploy worker unit
				if (_pTarget) {
					INTPOINT from(_mappos.x + rand() % 10 - 5, _mappos.y + rand() % 10 - 5);
					INTPOINT p = GetAttackPos(from);

					if (!_pTerrain->Within(p))
						p = _pTerrain->GetClosestFreeTile(_mappos, from);

					UNIT* punit = (UNIT*)_pTarget;
					punit->MoveUnit(p);
					punit->_position = _pTerrain->GetWorldPos(p);
					_pPlayer->_mapObjects.push_back(punit);
					_pTarget = nullptr;
				}
			}
		}
		else {
			if (_training) {
				_pTrainingEffect->Update(deltaTime);
				float progress = _trainingTime / _maxTrainingTime;
				_pTrainingEffect->Set(progress, _position + vec3(0.f, _BBox.max.y - _BBox.min.y + 1.5f, 0.f));

				_trainingTime += deltaTime;

				if (_trainingTime >= _maxTrainingTime) {
					_training = false;
					INTPOINT from(_mappos.x + rand() % 10 - 5, _mappos.y + rand() % 10 - 5);
					INTPOINT p = GetAttackPos(from);
					if (!_pTerrain->Within(p)) {
						p = _pTerrain->GetClosestFreeTile(_mappos, from);
					}
					UNIT* punit = (UNIT*)_pPlayer->AddMapObject(_trainingUnit, p, false, true);
					punit->Goto(_pTerrain->GetClosestFreeTile(punit->_mappos, punit->_mappos), false, true, STATE_MOVING);
				}
			}
		}
	}
	//update fires
	for (int i = 0; i < _fires.size(); i++) {
		if (_fires[i]) {
			_fires[i]->Update(deltaTime);
		}
	}
}

BBOX BUILDING::GetBoundingBox()
{
	return _BBox;
}

mat4 BUILDING::GetWorldMatrix()
{
	return _meshInstance.GetWorldMatrix();
}

bool BUILDING::isDead() {
	return _deathCountDown > 10.f;
}

void BUILDING::Damage(int dmg, MAPOBJECT* pattacker) {
	if (_dead)
		return;

	int oldHP = _hp;
	_hp -= dmg;

	//Add fires
	int limit[3] = { _hpMax * 0.75f, _hpMax * 0.5f, _hpMax * 0.25f };
	for (int i = 0; i < 3; i++) {
		if (_hp < limit[i] && oldHP >= limit[i]) {
			_fires.push_back(new EFFECT_FIRE(_pdevice, _firePos[i], _fireScale[i]));
		}
	}

	if (_hp <= 0) {

		_hp = 0;
		_dead = true;

		UNIT* pattackUnit = (UNIT*)pattacker;

		if (pattackUnit && pattackUnit->_pPlayer)
			pattackUnit->_pPlayer->_numKills++;

		//restore the tiles of the terrain
		if (_pTerrain && _affectTerrain) {
			Rect mr = GetMapRect(0);

			for (int y = mr.top; y <= mr.bottom; y++) {
				for (int x = mr.left; x <= mr.right; x++) {
					MAPTILE* ptile = _pTerrain->GetTile(x, y);
					if (ptile) {
						ptile->_walkable = true;
						ptile->_pMapObject = nullptr;
					}
				}
			}
			Rect r = GetMapRect(1);
			_pTerrain->UpdatePathfinding(&r);
			_pTerrain->_updateSight = true;
		}

		//kill fires
		for (int i = 0; i < _fires.size(); i++) {
			if (_fires[i])
				_fires[i]->Kill();
		}

		//Leave group
		if (_pGroup) {
			_pGroup->RemoveMember(this);
			_pGroup = nullptr;
		}
	}
}


void BUILDING::TrainUnit(int unit) {
	if (_pPlayer->money < GetCost(unit, false))
		return;

	_training = true;
	_trainingUnit = unit;
	_trainingTime = 0.f;
	_pPlayer->money -= GetCost(unit, false);

	if (unit == WORKER)
		_maxTrainingTime = 15.f;
	if (unit == SOLDIER)
		_maxTrainingTime = 20.f;
	if (unit == MAGICIAN)
		_maxTrainingTime = 30.f;

}