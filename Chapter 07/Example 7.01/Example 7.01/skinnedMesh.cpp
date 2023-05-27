#include "skinnedMesh.h"
#include <fstream>
int nextId = 0;
class BONE_HIERARCHY : public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME* ppNewFrame);
	STDMETHOD(CreateMeshContainer)(THIS_ LPCTSTR Name, CONST D3DXMESHDATA* pMeshData, CONST D3DXMATERIAL* pMaterials, CONST D3DXEFFECTINSTANCE* pEffectInstances, DWORD NumMaterials, CONST DWORD* pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER* ppNewMeshContainer);
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

HRESULT BONE_HIERARCHY::CreateFrame(LPCSTR Name, LPD3DXFRAME* ppNewFrame)
{
	BONE* newBone = new BONE;
	memset(newBone, 0, sizeof(BONE));
	newBone->id = nextId++;
	//Copy name
	if (Name != NULL)
	{
		newBone->Name = new char[strlen(Name) + 1];
		strcpy(newBone->Name, Name);		
	}

	//Set the transformation matrices
	D3DXMatrixIdentity(&newBone->TransformationMatrix);
	D3DXMatrixIdentity(&newBone->CombinedTransformationMatrix);

	//Return the new bone...
	*ppNewFrame = (D3DXFRAME*)newBone;

	return S_OK;
}

HRESULT BONE_HIERARCHY::CreateMeshContainer(LPCSTR Name,
	CONST D3DXMESHDATA* pMeshData,
	CONST D3DXMATERIAL* pMaterials,
	CONST D3DXEFFECTINSTANCE* pEffectInstances,
	DWORD NumMaterials,
	CONST DWORD* pAdjacency,
	LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER* ppNewMeshContainer)
{
	//Just return a temporary mesh for now...
	*ppNewMeshContainer = new BONEMESH;
	memset(*ppNewMeshContainer, 0, sizeof(BONEMESH));

	return S_OK;
}

HRESULT BONE_HIERARCHY::DestroyFrame(LPD3DXFRAME pFrameToFree)
{
	if (pFrameToFree)
	{
		if (pFrameToFree->Name != NULL)
			delete[] pFrameToFree->Name;
		delete pFrameToFree;
	}
	pFrameToFree = NULL;

	return S_OK;
}

HRESULT BONE_HIERARCHY::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	if (pMeshContainerBase != NULL)
		delete pMeshContainerBase;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//									SKINNED MESH												//
//////////////////////////////////////////////////////////////////////////////////////////////////

struct VERTEX {
	VERTEX();
	VERTEX(D3DXVECTOR3 pos, D3DCOLOR col) { position = pos; color = col; }
	D3DXVECTOR3 position;
	D3DCOLOR color;
	static const DWORD FVF;
};

const DWORD VERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

SKINNEDMESH::SKINNEDMESH()
{
	m_pRootBone = NULL;
	m_pSphereMesh = NULL;
}

SKINNEDMESH::~SKINNEDMESH()
{
	BONE_HIERARCHY boneHierarchy;
	boneHierarchy.DestroyFrame(m_pRootBone);
}



void SKINNEDMESH::Load(const char fileName[], IDirect3DDevice9* Dev)
{
	m_pDevice = Dev;

	BONE_HIERARCHY boneHierarchy;

	D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED, m_pDevice, &boneHierarchy,
		NULL, &m_pRootBone, NULL);

	D3DXMATRIX i;
	UpdateMatrices((BONE*)m_pRootBone, D3DXMatrixIdentity(&i));

	//Create Sphere
	D3DXCreateSphere(m_pDevice, 0.07f, 10, 10, &m_pSphereMesh, NULL);
	
}

void SKINNEDMESH::UpdateMatrices(BONE* bone, D3DXMATRIX* parentMatrix)
{
	if (bone == NULL)return;

	D3DXMatrixMultiply(&bone->CombinedTransformationMatrix,
		&bone->TransformationMatrix,
		parentMatrix);

	if (bone->pFrameSibling)UpdateMatrices((BONE*)bone->pFrameSibling, parentMatrix);
	if (bone->pFrameFirstChild)UpdateMatrices((BONE*)bone->pFrameFirstChild, &bone->CombinedTransformationMatrix);
}

void SKINNEDMESH::RenderSkeleton(BONE* bone, BONE* parent, D3DXMATRIX world)
{
	//Temporary function to render the bony hierarchy
	if (world == NULL)return;
	if (bone == NULL)bone = (BONE*)m_pRootBone;

	D3DXMATRIX r, s;
	D3DXMatrixRotationYawPitchRoll(&r, -D3DX_PI * 0.5f, 0.0f, 0.0f);

	//Draw Sphere
	m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
	D3DXMATRIX mat = (r * bone->CombinedTransformationMatrix * world);
	m_pDevice->SetTransform(D3DTS_WORLD, &mat);
	if (bone->Name != NULL)
		m_pSphereMesh->DrawSubset(0);

	//Draw line between bones
	if (parent != NULL && bone->Name != NULL && parent->Name != NULL)
	{
		D3DXMATRIX w1 = bone->CombinedTransformationMatrix;
		D3DXMATRIX w2 = parent->CombinedTransformationMatrix;

		//Extract translation
		D3DXVECTOR3 thisBone = D3DXVECTOR3(w1(3, 0), w1(3, 1), w1(3, 2));
		D3DXVECTOR3 ParentBone = D3DXVECTOR3(w2(3, 0), w2(3, 1), w2(3, 2));
		D3DXVECTOR3 vec = (thisBone - ParentBone);
		if (D3DXVec3Length(&vec) < 2.0f)
		{
			m_pDevice->SetTransform(D3DTS_WORLD, &world);
			VERTEX vert[] = { VERTEX(ParentBone, 0xffff0000), VERTEX(thisBone, 0xff00ff00) };
			m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
			m_pDevice->SetFVF(VERTEX::FVF);
			m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &vert[0], sizeof(VERTEX));
		}
	}

	if (bone->pFrameSibling)RenderSkeleton((BONE*)bone->pFrameSibling, parent, world);
	if (bone->pFrameFirstChild)RenderSkeleton((BONE*)bone->pFrameFirstChild, bone, world);
}