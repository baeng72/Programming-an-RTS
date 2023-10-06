#pragma once
#include<string>
#include "MeshTypes.h"
#include "../Renderer/RenderDevice.h"
#include "Mesh.h"
#include "MultiMesh.h"
#include "ProgressiveMesh.h"
#include "AnimatedMesh.h"
#include "../Renderer/Texture.h"


namespace Mesh {
	
	class Model {		
	public:
		static Model * Create(Renderer::RenderDevice* pdevice,const char * pmodelPath);
		virtual ~Model() = default;
		virtual uint32_t GetMeshCount() = 0;
		virtual Mesh* GetMesh(MeshType meshType,uint32_t i) = 0;
		virtual ProgressiveMesh* GetProgressiveMesh(MeshType meshType, uint32_t i) = 0;
		virtual float* GetMeshRawVertices(uint32_t i,uint32_t &stride,uint32_t&count ) = 0;
		virtual uint32_t* GetMeshRawIndices(uint32_t i, uint32_t& count) = 0;
		virtual glm::mat4 GetMeshXForm(uint32_t i) = 0;
		virtual uint32_t GetMeshMaterialIndex(uint32_t i) = 0;
		virtual uint32_t GetTextureCount(uint32_t matId,TextureType type) = 0;
		virtual Renderer::Texture* GetTexture(uint32_t matId,TextureType type, uint32_t i) = 0;
		virtual uint32_t GetMaterialCount() = 0;
		virtual Material* GetMaterial(uint32_t i) = 0;
		virtual uint32_t GetBoneCount(uint32_t i) = 0;
		virtual void GetBoneNames(uint32_t i,std::vector<std::string>& boneNames)=0;
		virtual void GetBoneXForms(uint32_t i, std::vector<mat4>& boneXForms) = 0;
		virtual void GetBoneInvBindXForms(uint32_t i, std::vector<mat4>& invBindXForms) = 0;
		virtual void GetBoneHierarchy(uint32_t i,std::vector<int>& boneHierarchy) = 0;
		virtual uint32_t GetAnimationCount(uint32_t i) = 0;
		virtual void GetAnimation(uint32_t i, uint32_t aniIdx, AnimationClip& animation)=0;
		virtual AnimatedMesh* GetAnimatedMesh(MeshType, uint32_t i) = 0;
		virtual MultiMesh* GetMultiMesh(MeshType) = 0;
		virtual void GetMultiMeshTextures(std::vector<Renderer::Texture*>& textures) = 0;
	};
}
