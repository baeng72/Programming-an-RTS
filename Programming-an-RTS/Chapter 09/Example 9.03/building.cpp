#include "building.h"

std::vector< std::unique_ptr<MESH>> buildingMeshes;

void LoadBuildingResources(Renderer::RenderDevice* pdevice) {
	std::vector<std::string> fnames = {
		"meshes/townhall.x",
		"meshes/barracks.x",
		"meshes/tower.x"
	};
	for (int i = 0; i < fnames.size(); i++) {
		auto& fname = fnames[i];
		std::string path = "../../../../Resources/Chapter 09/Example 9.03/" + fname;
		buildingMeshes.push_back(std::make_unique<MESH>(pdevice, path.c_str()));
	}
}

void UnloadBuildingResources() {
	buildingMeshes.clear();
}

bool PlaceOk(int buildType, INTPOINT mp, TERRAIN* pterrain) {
	if (pterrain == nullptr)
		return false;

	BUILDING b(buildType,0, mp, nullptr, false);
	Rect r = b.GetMapRect(1);

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

BUILDING::BUILDING(int type,int team, INTPOINT mp, TERRAIN* terrain, bool affectTerrain)
{
	_isBuilding = true;
	_type = type;
	_team = team;
	_mappos = mp;
	_pTerrain = terrain;
	_affectTerrain = affectTerrain;
	_team = 0;
	_range = _damage = 0;
	_meshInstance.SetMesh(buildingMeshes[_type].get());

	if (_type==0) {
		//Townhall
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
	else if (_type == 2) {
		//Tower
		_hp = _hpMax = 750;
		_sightRadius = 15;
		_name = "Tower";
		_mapsize.Set(2, 2);
		_meshInstance.SetScale(vec3(0.13f));
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
				if (ptile)
					ptile->_walkable = false;
			}
		}
		_pTerrain->UpdatePathfinding(&GetMapRect(1));
	}
	
}

BUILDING::~BUILDING()
{
	//restore the tiles of the terrain
	if (_pTerrain && _affectTerrain) {
		Rect mr = GetMapRect(0);
		for (int y = mr.top; y <= mr.bottom; y++) {
			for (int x = mr.left; x <= mr.right; x++) {
				MAPTILE* ptile = _pTerrain->GetTile(x, y);
				if (ptile)
					ptile->_walkable = true;
			}
		}
		_pTerrain->UpdatePathfinding(&GetMapRect(1));
	}
}

void BUILDING::Render(Renderer::Shader*pshader)
{
	mat4 matWorld = _meshInstance.GetWorldMatrix();
	mat4 xform = _meshInstance.GetMeshXForm();
	struct PushConst {
		mat4 world;
		vec4 teamColor;
		vec4 color;

	}pushConst = { matWorld * xform,vec4(1.f,0.f,0.f,1.f),vec4(1.f)};
	pshader->SetPushConstData(&pushConst, sizeof(pushConst));
	
	_meshInstance.Render(pshader);
}

void BUILDING::Update(float deltaTime)
{
	//Train units, upgrade things etc here
}

BBOX BUILDING::GetBoundingBox()
{
	return _BBox;
}

mat4 BUILDING::GetWorldMatrix()
{
	return _meshInstance.GetWorldMatrix();
}
