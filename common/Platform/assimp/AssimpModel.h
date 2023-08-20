#pragma once
#include <vector>
#include <memory>
#include "../../Renderer/Model.h"
#include "../../Renderer/Mesh.h"
#include "../../Renderer/Shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Assimp {
	class AssimpModel : public Renderer::Model {
		Renderer::RenderDevice* _pdevice;
		
		glm::mat4 _xform;
		std::string _path;
		struct AssimpMaterial {
			std::string name;
			glm::vec4 ambient;
			glm::vec4 diffuse;
			glm::vec4 specular;
			glm::vec4 emissive;
			std::vector<std::string> diffuseTextures;
			std::vector<std::string> normalTextures;
			std::vector < std::string> specularTextures;
		};

		std::vector<std::string> _diffuseTextures;

		struct AssimpVertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			AssimpVertex() {
				position = normal = glm::vec3(0.f);
				uv = glm::vec2(0.f);
			}
			AssimpVertex(glm::vec3& pos, glm::vec3& norm, glm::vec2& tex) {
				position = pos;
				normal = norm;
				uv = tex;
			}
		};
		struct AssimpPrimitive {
			std::string name;
			std::vector<AssimpVertex> vertices;
			std::vector<uint32_t> indices;	
			uint32_t materialIndex;
		};
		std::vector<AssimpMaterial> _materials;

		
		std::vector<AssimpPrimitive> _primitives;
		uint32_t _materialIndex;
		struct AssimpNode {
			std::string name;
			glm::mat4 nodeXForm;
			std::vector<AssimpNode> children;
		};
		AssimpNode _rootNode;
		glm::mat4 AssimpToGLM(aiMatrix4x4& mat);
		void ProcessMaterials(const aiScene* pscene);
		void ProcessNode(aiNode* pnode, const aiScene* pscene, glm::mat4& parentXForm, AssimpNode* currentNode);
		void ProcessMesh(aiMesh* pmesh, const aiScene* pscene);
		void ProcessTextureTypes(aiMaterial* pmat, aiTextureType type, std::vector<std::string>& textureNames);

	public:
		AssimpModel(Renderer::RenderDevice* device,const char*pmodelPath);
		virtual ~AssimpModel();
		virtual uint32_t GetMeshCount() override;
		virtual Renderer::Mesh* GetMesh(Renderer::MeshType meshType,uint32_t i) override;
		virtual glm::mat4 GetMeshXForm(uint32_t i) override;
		virtual uint32_t GetTextureCount(Renderer::TextureType type) override;
		virtual Renderer::Texture* GetTexture(Renderer::TextureType type, uint32_t i) override;
		virtual uint32_t GetMaterialCount() override;
		virtual Renderer::Material* GetMaterial(uint32_t i) override;
	};
}
