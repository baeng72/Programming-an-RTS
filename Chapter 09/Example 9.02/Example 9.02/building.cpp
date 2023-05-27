#include "building.h"

std::vector<MESH*> buildingMeshes;

void LoadBuildingResources(IDirect3DDevice9* Device)
{
	std::vector<std::string> fnames;

	fnames.push_back("meshes/townhall.x");
	fnames.push_back("meshes/barracks.x");
	fnames.push_back("meshes/tower.x");

	for (int i = 0; i < fnames.size(); i++)
		buildingMeshes.push_back(new MESH((char*)fnames[i].c_str(), Device));
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

	BUILDING b(buildType, mp, NULL, false, NULL);
	RECT r = b.GetMapRect(1);

	for (int y = r.top; y <= r.bottom; y++)
		for (int x = r.left; x <= r.right; x++)
		{
			//Building must be within map borders
			if (!terrain->Within(INTPOINT(x, y)))return false;
			MAPTILE* tile = terrain->GetTile(x, y);
			if (tile == NULL)return false;

			//The terrain must be level and walkable
			if (tile->m_height != 0.0f || !tile->m_walkable)return false;
		}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//								BUILDING										//
//////////////////////////////////////////////////////////////////////////////////

BUILDING::BUILDING(int _type, INTPOINT mp, TERRAIN* _terrain, bool _affectTerrain, IDirect3DDevice9* Dev)
{
	m_type = _type;
	m_mappos = mp;
	m_pTerrain = _terrain;
	m_affectTerrain = _affectTerrain;
	m_pDevice = Dev;
	m_team = 0;
	m_range = m_damage = 0;
	m_meshInstance.SetMesh(buildingMeshes[m_type]);

	if (m_type == 0)		//Townhall
	{
		m_hp = m_hpMax = 600;
		m_sightRadius = 10;
		m_name = "Townhall";
		m_mapsize.Set(4, 2);
		m_meshInstance.SetScale(D3DXVECTOR3(0.13f, 0.13f, 0.13f));
	}
	else if (m_type == 1)		//Barracks
	{
		m_hp = m_hpMax = 450;
		m_sightRadius = 8;
		m_name = "Barracks";
		m_mapsize.Set(2, 4);
		m_meshInstance.SetScale(D3DXVECTOR3(0.15f, 0.15f, 0.15f));
	}
	else if (m_type == 2)		//Tower
	{
		m_hp = m_hpMax = 750;
		m_sightRadius = 15;
		m_name = "Tower";
		m_mapsize.Set(2, 2);
		m_meshInstance.SetScale(D3DXVECTOR3(0.13f, 0.13f, 0.13f));
	}

	if (m_pTerrain != NULL)
		m_position = m_pTerrain->GetWorldPos(m_mappos) + D3DXVECTOR3(m_mapsize.x / 2.0f, 0.0f, -m_mapsize.y / 2.0f);
	else m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_meshInstance.SetPosition(m_position);

	m_BBox = m_meshInstance.GetBoundingBox();
	m_BBox.max -= D3DXVECTOR3(0.2f, 0.2f, 0.2f);
	m_BBox.min += D3DXVECTOR3(0.2f, 0.2f, 0.2f);

	//Update the tiles of the terrain
	if (m_pTerrain != NULL && m_affectTerrain)
	{
		RECT mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++)
			for (int x = mr.left; x <= mr.right; x++)
			{
				MAPTILE* tile = m_pTerrain->GetTile(x, y);
				if (tile != NULL)tile->m_walkable = false;
			}
		RECT r = GetMapRect(1);
		m_pTerrain->UpdatePathfinding(&r);
	}
}

BUILDING::~BUILDING()
{
	//restore the tiles of the terrain
	if (m_pTerrain != NULL && m_affectTerrain)
	{
		RECT mr = GetMapRect(0);

		for (int y = mr.top; y <= mr.bottom; y++)
			for (int x = mr.left; x <= mr.right; x++)
			{
				MAPTILE* tile = m_pTerrain->GetTile(x, y);
				if (tile != NULL)tile->m_walkable = true;
			}
		RECT r = GetMapRect(1);
		m_pTerrain->UpdatePathfinding(&r);
	}
}

void BUILDING::Render()
{
	m_meshInstance.Render();
}

void BUILDING::Update(float deltaTime)
{
	//Train units, upgrade things etc here...
}

BBOX BUILDING::GetBoundingBox()
{
	return m_BBox;
}

D3DXMATRIX BUILDING::GetWorldMatrix()
{
	return m_meshInstance.GetWorldMatrix();
}