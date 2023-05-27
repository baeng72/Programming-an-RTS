#include "object.h"

std::vector<MESH*> objectMeshes;

HRESULT LoadObjectResources(IDirect3DDevice9* Device)
{
	MESH* mech1 = new MESH("Objects/mech1.x", Device);
	objectMeshes.push_back(mech1);

	MESH* mech2 = new MESH("Objects/mech2.x", Device);
	objectMeshes.push_back(mech2);

	MESH* mech3 = new MESH("Objects/mech3.x", Device);
	objectMeshes.push_back(mech3);

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

OBJECT::OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	m_type = t;

	for (int i = 0; i < 3; i++)
	{
		m_meshInstances[i].SetPosition(pos);
		m_meshInstances[i].SetRotation(rot);
		m_meshInstances[i].SetScale(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_meshInstances[i].SetMesh(objectMeshes[m_type + i]);
	}

	m_BBox = m_meshInstances[1].GetBoundingBox();
}

void OBJECT::Render(CAMERA* camera, long& noFaces, int& noObjects)
{
	//If camera == NULL, Render High Res Mesh
	if (camera == NULL)
	{
		m_meshInstances[0].Render();
		noFaces += m_meshInstances[0].m_pMesh->m_pMesh->GetNumFaces();
		noObjects++;
	}
	else
	{
		if (!camera->Cull(m_BBox))		//Cull objects
		{
			//Distance from objects to camera
			D3DXVECTOR3 vec = (m_meshInstances[0].m_pos - camera->m_eye);
			float dist = D3DXVec3Length(&vec);
			noObjects++;

			if (dist < 50.0f)		//Close to the Camera
			{
				m_meshInstances[0].Render();	//Render High Res
				noFaces += m_meshInstances[0].m_pMesh->m_pMesh->GetNumFaces();
			}
			else if (dist < 100.0f)	//Average distance from Camera
			{
				m_meshInstances[1].Render();	//Render Medium Res mesh
				noFaces += m_meshInstances[1].m_pMesh->m_pMesh->GetNumFaces();
			}
			else					//Far from camera
			{
				m_meshInstances[2].Render();	//Render Low Res Mesh
				noFaces += m_meshInstances[2].m_pMesh->m_pMesh->GetNumFaces();
			}
		}
	}
}