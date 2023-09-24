#include "building.h"

std::vector<MESH*> buildingMeshes;

void LoadBuildingResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager) {
	std::vector<std::string> fnames = {
		"meshes/townhall.x",
		"meshes/barracks.x",
		"meshes/tower.x"
	};

	for (auto& fname : fnames) {
		std::string path = "../../../../Resources/Chapter 09/Example 9.01/" + fname;
		buildingMeshes.push_back(new MESH(pdevice, shaderManager, path.c_str()));
	}
	
}

void UnloadBuildingResources() {
	for (auto& building : buildingMeshes) {
		delete building;
	}
	buildingMeshes.clear();
}

bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* terrain) {
	if (!terrain)
		return false;

	BUILDING b(buildType, 0, mp, nullptr, false, nullptr);
	Rect r = b.GetMapRect(1);

	for (int y = r.top; y <= r.bottom; y++) {
		for (int x = r.left; x <= r.right; x++) {
			//Building must be within map borders
			if (!terrain->Within(INTPOINT(x, y)))
				return false;
			MAPTILE* tile = terrain->GetTile(x, y);
			if (tile == nullptr)
				return false;

			//The terrain must be level and walkable
			if (tile->_height != 0.f || !tile->_walkable)
				return false;
		}
	}
	return true;
}

///// BUILDING

BUILDING::BUILDING(int type, int team, INTPOINT mp, TERRAIN* terrain, bool affectTerrain, Renderer::RenderDevice* pdevice) {
	_type = type;
	_team = team;
	_mappos = mp;
	_pTerrain = terrain;
	_affectTerrain = affectTerrain;
	_pDevice = pdevice;
	_team = 0;
	_range = _damage = 0;
	_meshInstance.SetMesh(buildingMeshes[type]);
	_isBuilding = true;

	if (_type == 0)//townhall
	{
		_hp = _hpMax = 600;
		_sightRadius = 10;
		_name = "Townhall";
		_mapsize.Set(4, 2);
		_meshInstance.SetScale(vec3(0.13f));
	}
	else if (_type == 1) {
		//Barracks
		_hp = _hpMax = 450;
		_sightRadius = 8;
		_name = "Barracks";
		_mapsize.Set(2, 4);
		_meshInstance.SetScale(vec3(0.15f));			
	}
	else if (_type == 2) {//Tower
		_hp = _hpMax = 750;
		_sightRadius = 15;
		_name = "Tower";
		_mapsize.Set(2, 2);
		_meshInstance.SetScale(vec3(0.13f));
	}

	if (_pTerrain) {
		_position = _pTerrain->GetWorldPos(_mappos) + vec3(_mapsize.x / 2.f, 0.f, -_mapsize.y / 2.f);
	}
	else {
		_position = vec3(0.f);
	}

	_meshInstance.SetPosition(_position);

	_BBox = _meshInstance.GetBoundingBox();
	_BBox.max -= vec3(0.2f);
	_BBox.min += vec3(0.2f);

	//Update the tiles of the _pTerrain
	if (_pTerrain != nullptr && _affectTerrain) {
		Rect mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++) {
			for (int x = mr.left; x <= mr.right; x++) {
				MAPTILE* tile = _pTerrain->GetTile(x, y);
				if (tile != nullptr) {
					tile->_walkable = false;
					tile->_pMapObject = this;
				}
			}
		}
		_pTerrain->UpdatePathfinding(&GetMapRect(1));
	}
}

BUILDING::~BUILDING() {
	//restore the tiles of the terrain
	if (_pTerrain != nullptr && _affectTerrain) {
		Rect mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++) {
			for (int x = mr.left; x <= mr.right; x++) {
				MAPTILE* tile = _pTerrain->GetTile(x, y);
				if (tile) {
					tile->_walkable = true;
					tile->_pMapObject = nullptr;
				}
			}
		}
		_pTerrain->UpdatePathfinding(&GetMapRect(1));
	}
}

void BUILDING::Render(mat4&matVP,Renderer::DirectionalLight&light) {
	_meshInstance.Render(matVP, light);
}

void BUILDING::Update(float deltaTime) {
	//train units, upgrade things here
}

BBOX BUILDING::GetBoundingBox() {
	return _BBox;
}

mat4 BUILDING::GetWorldMatrix() {
	return _meshInstance.GetWorldMatrix();
}