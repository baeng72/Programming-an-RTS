#pragma once
#include <vector>
#include <memory>
#include "../../Mesh/Model.h"
#include "../../Mesh/Mesh.h"
#include "../../mesh/MultiMesh.h"
#include "../../mesh/ProgressiveMesh.h"
#include "../../Renderer/Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Assimp {
	class AssimpModel : public Mesh::Model {
		Renderer::RenderDevice* _pdevice;
		
		glm::mat4 _xform;
		std::string _path;
		

		

		struct AssimpVertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::ivec4 boneIDs;
			glm::vec4 weights;
			AssimpVertex() {
				position = normal = glm::vec3(0.f);
				uv = glm::vec2(0.f);
				tangent = bitangent = vec3(0.f);
				boneIDs = ivec4(-1);
				weights = vec4(0.f);
				
			}
			AssimpVertex(glm::vec3& pos, glm::vec3& norm, glm::vec2& tex,vec3&tan,vec3&bitan,ivec4&bones,vec4&ws) {
				position = pos;
				normal = norm;
				uv = tex;
				tangent = tan;
				bitangent = bitan;
				boneIDs = bones;
				weights = ws;
			}

		};
		struct AssimpPrimitive {
			std::string name;
			std::vector<AssimpVertex> vertices;
			std::vector<uint32_t> indices;	
			uint32_t materialIndex;
		};
		std::vector<Mesh::Material> _materials;

		
		std::vector<AssimpPrimitive> _primitives;
		std::vector<uint32_t> _materialIndices;
		struct AssimpNode {
			std::string name;
			mat4 localXForm;
			glm::mat4 nodeXForm;
			std::vector<AssimpNode> children;
		};
		AssimpNode _rootNode;
		//struct BoneInfo {
		//	int32_t id;//id in bone matrices
		//	int32_t parentID;
		//	glm::mat4 xform;//local transformation
		//	glm::mat4 offset;//offset matri transforms vertices from model to bone space
		//};
		//
		std::unordered_map<std::string, mat4> _boneMap;
		int _boneCount;
		//std::vector<std::string> _nameList;
		std::vector<int> _boneHierarchy;
		std::vector<mat4> _boneOffsets;
		std::vector<mat4> _boneXForms;
		std::vector<std::string> _boneNames;
		std::vector<Mesh::Skeleton> _skeletons;
		std::vector<Mesh::AnimationClip> _animations;
		
		void ProcessMaterials(const aiScene* pscene);
		void ProcessNode(aiNode* pnode, const aiScene* pscene, glm::mat4& parentXForm, AssimpNode* currentNode);
		void ProcessMesh(aiMesh* pmesh, const aiScene* pscene);
		void ProcessBoneHierarchy(AssimpNode& node, int parentID);
		void ProcessTextureTypes(aiMaterial* pmat, aiTextureType type, std::vector<std::string>& textureNames);
		void ProcessAnimations(const aiScene* pscene);
		
		//conversion helpers
		glm::mat4 AssimpToGLM(aiMatrix4x4& mat);
		vec3 AssimpToGLM(aiVector3D& vec);
		quat AssimpToGLM(aiQuaternion& qu);
	public:
		AssimpModel(Renderer::RenderDevice* device,const char*pmodelPath);
		virtual ~AssimpModel();
		virtual uint32_t GetMeshCount() override;
		virtual Mesh::Mesh* GetMesh(Mesh::MeshType meshType,uint32_t i) override;
		virtual Mesh::ProgressiveMesh* GetProgressiveMesh(Mesh::MeshType meshType, uint32_t i) override;
		virtual float* GetMeshRawVertices(uint32_t i, uint32_t &stride, uint32_t& count) override;
		virtual uint32_t* GetMeshRawIndices(uint32_t i, uint32_t& count) override;
		virtual glm::mat4 GetMeshXForm(uint32_t i) override;
		virtual uint32_t GetMeshMaterialIndex(uint32_t i) override;
		virtual uint32_t GetTextureCount(uint32_t matId,Mesh::TextureType type) override;
		virtual Renderer::Texture* GetTexture(uint32_t matId,Mesh::TextureType type, uint32_t i) override;
		virtual uint32_t GetMaterialCount() override;
		virtual Mesh::Material* GetMaterial(uint32_t i) override;
		virtual uint32_t GetBoneCount(uint32_t i) override;
		virtual void GetBoneNames(uint32_t i,std::vector<std::string>& boneNames) override;
		virtual void GetBoneXForms(uint32_t i,std::vector<mat4>& boneXForms)override;
		virtual void GetBoneInvBindXForms(uint32_t i, std::vector<mat4>& invBindXForms) override;
		virtual void GetBoneHierarchy(uint32_t i,std::vector<int>& boneHierarchy) override;
		virtual uint32_t GetAnimationCount(uint32_t i) override;
		virtual void GetAnimation(uint32_t i, uint32_t aniIdx, Mesh::AnimationClip& animation) override;
		virtual Mesh::AnimatedMesh* GetAnimatedMesh(Mesh::MeshType, uint32_t i)override;
		virtual void GetAnimatedMeshData(uint32_t i, float** ppvertices, uint32_t& vertSize, uint32_t** ppindices, uint32_t& indSize, Renderer::VertexAttributes& attributes, Mesh::Skeleton& skeleton, std::vector<Mesh::AnimationClip>& animations) override;
		virtual Mesh::MultiMesh* GetMultiMesh(Mesh::MeshType type) override;
		virtual void GetMultiMeshTextures(std::vector<Renderer::Texture*>& textures)override;
	};
}
