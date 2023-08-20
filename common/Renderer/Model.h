#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

namespace Renderer {
	enum class MeshType{position,position_normal,position_normal_uv};
	enum class TextureType{diffuse};
	class Model {		
	public:
		static Model * Create(RenderDevice* pdevice,const char * pmodelPath);
		virtual ~Model() = default;
		virtual uint32_t GetMeshCount() = 0;
		virtual Mesh* GetMesh(MeshType meshType,uint32_t i) = 0;
		virtual glm::mat4 GetMeshXForm(uint32_t i) = 0;
		virtual uint32_t GetTextureCount(TextureType type) = 0;
		virtual Texture* GetTexture(TextureType type, uint32_t i) = 0;
		virtual uint32_t GetMaterialCount() = 0;
		virtual Material* GetMaterial(uint32_t i) = 0;
	};
}
