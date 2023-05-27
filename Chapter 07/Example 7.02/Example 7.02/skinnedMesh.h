#ifndef SKINNED_MESH
#define SKINNED_MESH

#include <windows.h>
#include <d3dx9.h>
#include <string>
#include <vector>
#include "debug.h"

struct BONE : public D3DXFRAME
{
	D3DXMATRIX CombinedTransformationMatrix;
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
	void SetPose(D3DXMATRIX world, ID3DXAnimationController* animControl, float time);
	void SetAnimation(const char name[]);
	std::vector<std::string> GetAnimations();

private:

	void UpdateMatrices(BONE* bone, D3DXMATRIX* parentMatrix);

	IDirect3DDevice9* m_pDevice;
	D3DXFRAME* m_pRootBone;
	ID3DXAnimationController* m_pAnimControl;

	LPD3DXMESH m_pSphereMesh;
};

#endif