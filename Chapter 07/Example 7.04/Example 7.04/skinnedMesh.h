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
	ID3DXMesh* OriginalMesh;
	std::vector<D3DMATERIAL9> materials;
	std::vector<IDirect3DTexture9*> textures;

	DWORD NumAttributeGroups;
	D3DXATTRIBUTERANGE* attributeTable;
	D3DXMATRIX** boneMatrixPtrs;
	D3DXMATRIX* boneOffsetMatrices;
	D3DXMATRIX* currentBoneMatrices;
};

class SKINNEDMESH
{
public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(const char fileName[], IDirect3DDevice9* Dev);
	void Render(BONE* bone);
	void SetPose(D3DXMATRIX world, ID3DXAnimationController* animControl, float time);
	void SetAnimation(const char name[]);
	std::vector<std::string> GetAnimations();
	BONE* FindBone(const char name[]);

private:
	void UpdateMatrices(BONE* bone, D3DXMATRIX* parentMatrix);
	void SetupBoneMatrixPointers(BONE* bone);

	IDirect3DDevice9* m_pDevice;
	D3DXFRAME* m_pRootBone;
	ID3DXAnimationController* m_pAnimControl;
};

#endif