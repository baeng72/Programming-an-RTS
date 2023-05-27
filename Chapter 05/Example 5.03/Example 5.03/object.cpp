#include "object.h"

std::vector<MESH*> objectMeshes;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* dragon = new MESH("objects/dragon.x", Device);
	objectMeshes.push_back(dragon);

	MESH* f1 = new MESH("objects/footman01.x", Device);
	objectMeshes.push_back(f1);

	MESH* ring = new MESH("objects/ring.x", Device);
	objectMeshes.push_back(ring);

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
	m_type = 0;
}

OBJECT::OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca)
{
	m_type = t;
	m_meshInstance.SetPosition(pos);
	m_meshInstance.SetRotation(rot);
	m_meshInstance.SetScale(sca);
	m_meshInstance.SetMesh(objectMeshes[m_type]);
	m_boxMesh = NULL;
	m_sphereMesh = NULL;

	if (m_type == DRAGON)
		m_name = "Dragon";
	else if (m_type == BOB)
		m_name = "Brave Bob";
	else if (m_type == RING)
		m_name = "";

	m_BBox = m_meshInstance.GetBoundingBox();
	m_BSphere = m_meshInstance.GetBoundingSphere();

	D3DXCreateBox(m_meshInstance.m_pMesh->m_pDevice, m_BBox.max.x - m_BBox.min.x,
		m_BBox.max.y - m_BBox.min.y,
		m_BBox.max.z - m_BBox.min.z,
		&m_boxMesh, NULL);

	D3DXCreateSphere(m_meshInstance.m_pMesh->m_pDevice, m_BSphere.radius, 20, 20, &m_sphereMesh, NULL);
}

void OBJECT::Render()
{
	m_meshInstance.Render();
}

void OBJECT::RenderBoundingVolume(int typ)
{
	D3DXMATRIX world;

	D3DMATERIAL9 mtrl;
	mtrl.Diffuse = D3DXCOLOR(0.5f, 0.5f, 1.0f, 0.1f);
	mtrl.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.1f);
	mtrl.Emissive = mtrl.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
	mtrl.Power = 0.2f;

	m_meshInstance.m_pMesh->m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	m_meshInstance.m_pMesh->m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	m_meshInstance.m_pMesh->m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	m_meshInstance.m_pMesh->m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

	m_meshInstance.m_pMesh->m_pDevice->SetMaterial(&mtrl);
	m_meshInstance.m_pMesh->m_pDevice->SetTexture(0, NULL);
	m_meshInstance.m_pMesh->m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	if (typ == 1)
	{
		D3DXVECTOR3 center = (m_BBox.max + m_BBox.min) / 2.0f;
		D3DXMatrixTranslation(&world, center.x, center.y, center.z);
		m_meshInstance.m_pMesh->m_pDevice->SetTransform(D3DTS_WORLD, &world);
		m_boxMesh->DrawSubset(0);
	}
	else if (typ == 2)
	{
		D3DXMatrixTranslation(&world, m_BSphere.center.x, m_BSphere.center.y, m_BSphere.center.z);
		m_meshInstance.m_pMesh->m_pDevice->SetTransform(D3DTS_WORLD, &world);
		m_sphereMesh->DrawSubset(0);
	}

	m_meshInstance.m_pMesh->m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}