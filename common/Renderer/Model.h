#pragma once
#include<string>
#include "../Core/defines.h"
#include "RenderDevice.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

namespace Renderer {
	struct ModelMaterial {
		std::string name;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 emissive;
		std::vector<std::string> diffuseTextures;
		std::vector<std::string> normalTextures;
		std::vector < std::string> specularTextures;
	};
	enum class MeshType{position,position_normal,position_normal_uv};
	enum class TextureType{diffuse};
	class Model {		
	public:
		static Model * Create(RenderDevice* pdevice,const char * pmodelPath);
		virtual ~Model() = default;
		virtual uint32_t GetMeshCount() = 0;
		virtual Mesh* GetMesh(MeshType meshType,uint32_t i) = 0;
		virtual glm::mat4 GetMeshXForm(uint32_t i) = 0;
		virtual uint32_t GetMeshMaterialIndex(uint32_t i) = 0;
		virtual uint32_t GetTextureCount(TextureType type) = 0;
		virtual Texture* GetTexture(TextureType type, uint32_t i) = 0;
		virtual uint32_t GetMaterialCount() = 0;
		virtual ModelMaterial* GetMaterial(uint32_t i) = 0;
	};
}
