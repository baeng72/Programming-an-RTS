#ifndef SKINNED_MESH
#define SKINNED_MESH

#include <windows.h>
#include <d3dx9.h>
#include <string>
#include <vector>
#include "debug.h"

#define ANIM_STILL 4
#define ANIM_IDLE 3
#define ANIM_RUN 2
#define ANIM_ATTACK 1
#define ANIM_DIE 0

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
	void Load(char fileName[], IDirect3DDevice9* Dev);
	void Render(BONE* bone);

	BONE* FindBone(char name[]);
	void SetPose(D3DXMATRIX world, ID3DXAnimationController* animControl, float time);
	int SetAnimation(char name[]);
	void SetAnimation(int index);
	float GetAnimationDuration(int index);
	std::vector<std::string> GetAnimations();
	ID3DXMesh* GetCurrentMesh(BONE* bone);

	ID3DXAnimationController* GetAnimationControl();

private:
	void UpdateMatrices(BONE* bone, D3DXMATRIX* parentMatrix);
	void SetupBoneMatrixPointers(BONE* bone);

	IDirect3DDevice9* m_pDevice;
	D3DXFRAME* m_pRootBone;
	ID3DXAnimationController* m_pAnimControl;
};

#endif