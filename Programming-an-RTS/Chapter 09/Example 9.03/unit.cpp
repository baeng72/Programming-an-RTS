#include "unit.h"

std::vector<std::unique_ptr<SKINNEDMESH>> unitMeshes;

void LoadUnitResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager) {
	std::vector<std::string> fnames = {
		"units/drone.x",
		"units/soldier.x",
		"units/magician.x"
	};
	unitMeshes.resize(fnames.size());
	for (int i = 0; i < fnames.size();i++) {
		auto& fname = fnames[i];
		std::string path = "../../../../Resources/Chapter 09/Example 9.01/"+fname;
		
		unitMeshes[i] = std::make_unique<SKINNEDMESH>();
		unitMeshes[i]->Load(pdevice, shaderManager, path.c_str());
		
		
	}
}

void UnloadUnitResources() {
	unitMeshes.clear();
}

// UINT

UNIT::UNIT(int type, int team, INTPOINT mp, TERRAIN* terrain, Renderer::RenderDevice* pdevice) {
	_type = type;
	_team = team;
	_mappos = mp;
	_pTerrain = terrain;
	_pDevice = pdevice;
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
	SetAnimation("Still");
}

UNIT::~UNIT() {
	if (_pTerrain) {
		MAPTILE* tile = _pTerrain->GetTile(_mappos);
		if (tile)
			tile->_pMapObject = nullptr;
	}
}