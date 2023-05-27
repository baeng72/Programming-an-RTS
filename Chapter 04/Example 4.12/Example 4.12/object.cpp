#include "object.h"

std::vector<MESH*> objectMeshes;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* tree = new MESH("meshes/tree.x", Device);
	objectMeshes.push_back(tree);

	MESH* stone = new MESH("meshes/stone.x", Device);
	objectMeshes.push_back(stone);

	return S_OK;
}

void UnloadObjectResources()
{
	for (int i = 0; i < objectMeshes.size(); i++)
		objectMeshes[i]->Release();

	objectMeshes.clear();
}

//////////////////////////////////////////////////////////////////////////////
//							OBJECT CLASS									//
//////////////////////////////////////////////////////////////////////////////

OBJECT::OBJECT()
{
	m_iType = 0;
}

OBJECT::OBJECT(int t, INTPOINT mp, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca)
{
	m_iType = t;
	m_mappos = mp;
	m_meshInstance.SetPosition(pos);
	m_meshInstance.SetRotation(rot);
	m_meshInstance.SetScale(sca);
	m_meshInstance.SetMesh(objectMeshes[m_iType]);
}

void OBJECT::Render()
{
	m_meshInstance.Render();
}
