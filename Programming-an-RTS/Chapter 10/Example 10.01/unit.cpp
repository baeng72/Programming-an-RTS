#include "unit.h"
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
		std::string path = "../../../../Resources/Chapter 09/Example 9.03/" + fname;

		unitMeshes[i] = std::make_unique<SKINNEDMESH>();
		unitMeshes[i]->Load(pdevice, shaderManager, path.c_str());


	}
}

void UnloadUnitResources() {
	unitMeshes.clear();
}



// UINT

UNIT::UNIT(int type, int team, INTPOINT mp, TERRAIN* terrain) {
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

	if (terrain) {
		_position = _pTerrain->GetWorldPos(_mappos);
		MAPTILE* tile = _pTerrain->GetTile(_mappos);
		if (tile)
			tile->_pMapObject = this;
	}
	else
		_position = vec3(0.f);

	if (_type == 0) {
		//farmer
		_hp = _hpMax = 100;
		_range = 1;
		_damage = 5;
		_sightRadius = 7;
		_speed = 1.f;
		_name = "Farmer";
	}
	else if (_type == 1) {
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
	SetAnimation("Still");
}

UNIT::~UNIT() {
	if (_pTerrain) {
		MAPTILE* tile = _pTerrain->GetTile(_mappos);
		if (tile)
			tile->_pMapObject = nullptr;
	}
}

void UNIT::Render(Renderer::Shader* pshader) {

	if (_type < unitMeshes.size() && unitMeshes[_type]) {
		{
			{

				SetAnimation(_animation);
			}
			{
				unitMeshes[_type]->SetPose(_time);
				_time = 0.f;
			}
		}
		mat4 worldxform = unitMeshes[_type]->GetWorldXForm() * GetWorldMatrix();
		vec4 color = vec4(1.f, 0.f, 0.f, 1.f);
		pshader->SetUniform("model", &worldxform);
		pshader->SetUniform("color", &color);
		//if (Core::GetAPI() == Core::API::Vulkan) {
		//	struct PushConst {
		//		mat4 world;
		//		vec4 color;
		//	}pushConst = { worldxform, color };
		//	//pshader->SetPushConstData(&pushConst, sizeof(pushConst));
		//	pshader->SetUniformData("PushConst", &pushConst, sizeof(pushConst));
		//}
		//else {
		//	pshader->SetUniformData("model", &worldxform, sizeof(mat4));
		//	pshader->SetUniformData("color", &color, sizeof(vec4));
		//}
		unitMeshes[_type]->Render(pshader);

	}
}

void UNIT::Update(float deltaTime) {
	//update unit animation time

	_time += deltaTime * 0.8f * _speed * 100;

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
			}
			else if (!CheckCollision(_path[_activeWP])) {//Next waypoint
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
	return false;//no collision
}

void UNIT::Goto(INTPOINT mp, bool considerUnits, bool finalGoal) {
	if (!_pTerrain) {
		return;
	}
	if (finalGoal)
		_finalGoal = mp;
	//clear old path
	_path.clear();
	_activeWP = 0;

	if (_moving) {	//if unit is currently moving
		//Finish the active waypoint
		_path.push_back(_mappos);
		std::vector<INTPOINT> tmpPath = _pTerrain->GetPath(_mappos, mp, considerUnits);
		//add new path
		_path.insert(_path.end(), tmpPath.begin(), tmpPath.end());
	}
	else {
		_path = _pTerrain->GetPath(_mappos, mp, considerUnits);

		if (_path.size()) {	//if a path was found
			_moving = true;
			//check that the next tile if free

			if (!CheckCollision(_path[_activeWP])) {
				MoveUnit(_path[_activeWP]);
				SetAnimation("Run");
			}
		}
	}
}

void UNIT::MoveUnit(INTPOINT to) {
	_lastWP = _pTerrain->GetWorldPos(_mappos);
	_rotation = GetDirection(_mappos, to);

	_mappos = to;
	_movePrc = 0.f;
	_nextWP = _pTerrain->GetWorldPos(_mappos);
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
	unitMeshes[_type]->SetAnimation(_animation);
}

void UNIT::SetAnimation(int i) {
	_animation = i;
	unitMeshes[_type]->SetAnimation(i);
}

Renderer::Shader* UNIT::GetShader() {
	return unitMeshes[_type]->GetShader();
}

