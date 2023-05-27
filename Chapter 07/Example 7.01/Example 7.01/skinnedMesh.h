#ifndef SKINNED_MESH
#define SKINNED_MESH

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"

struct BONE : public D3DXFRAME
{
	D3DXMATRIX CombinedTransformationMatrix;
	int id{ 0 };
};

struct BONEMESH : public D3DXMESHCONTAINER
{
	//More to come here later...
};

class SKINNEDMESH
{

public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(const char fileName[], IDirect3DDevice9* Dev);
	void RenderSkeleton(BONE* bone, BONE* parent, D3DXMATRIX world);

private:

	void UpdateMatrices(BONE* bone, D3DXMATRIX* parentMatrix);

	IDirect3DDevice9* m_pDevice;
	D3DXFRAME* m_pRootBone;
	LPD3DXMESH m_pSphereMesh;
};

#endif