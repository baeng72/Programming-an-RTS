#include "object.h"

std::vector<MESH*> objectMeshes;
ID3DXLine* line = NULL;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* tile = new MESH("Objects/tile.x", Device);
	objectMeshes.push_back(tile);

	MESH* house = new MESH("Objects/house.x", Device);
	objectMeshes.push_back(house);

	MESH* house2 = new MESH("Objects/house2.x", Device);
	objectMeshes.push_back(house2);

	MESH* park = new MESH("Objects/park.x", Device);
	objectMeshes.push_back(park);

	D3DXCreateLine(Device, &line);

	return S_OK;
}

void UnloadObjectResources()
{
	for (int i = 0; i < objectMeshes.size(); i++)
		objectMeshes[i]->Release();

	objectMeshes.clear();

	line->Release();
}

//////////////////////////////////////////////////////////////////////////////
//							OBJECT CLASS									//
//////////////////////////////////////////////////////////////////////////////

OBJECT::OBJECT()
{
	m_type = 0;
}

OBJECT::OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca)
{
	m_type = t;
	m_meshInstance.SetPosition(pos);
	m_meshInstance.SetRotation(rot);
	m_meshInstance.SetScale(sca);
	m_meshInstance.SetMesh(objectMeshes[m_type]);

	m_bBox = m_meshInstance.GetBoundingBox();
	m_bSphere = m_meshInstance.GetBoundingSphere();
}

void OBJECT::Render()
{
	m_meshInstance.Render();
}