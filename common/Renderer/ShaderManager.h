#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Light.h"
#include "Texture.h"
#include "Buffer.h"
namespace Renderer {
	enum class ShaderStorageType { Uniform, UniformDynamic, Storage, StorageDynamic, Texture, TextureArray };
	enum ShaderAttrFlagBits {
		SHADER_ATTR_UBO	= 0x0000001,		//uniform buffer
		SHADER_ATTR_STORAGE	= 0x000002,		//storage buffer
		SHADER_ATTR_SAMPLER	= 0x000004,		//image sampler
		SHADER_ATTR_SAMPLER_ARRAY = 0x000008,//array of sampler
	};
	using ShaderAttrFlags = uint32_t;
	enum ShaderStageFlagBits {
		SHADER_STAGE_VERTEX=0x000001,
		SHADER_STAGE_GEOMETRY=0x00002,
		SHADER_STAGE_FRAGMENT=0x00004,
	};
	
	using ShaderStageFlags = uint32_t;
	struct ShaderAttrData {
		const char* name;
		uint32_t flags;
		uint32_t stages;
		uint32_t size;
		uint32_t count;
	};
	
	struct FlatShaderUBO {
		glm::mat4 viewProj;		
	};
	struct FlatShaderDirectionalUBO : public FlatShaderUBO {
		Renderer::DirectionalLight light;
		FlatShaderDirectionalUBO(glm::mat4& vp, Renderer::DirectionalLight& l) {
			viewProj = vp;
			light = l;
		}
	};
	struct FlatShaderPushConst {		
		glm::mat4 model;
	};
	class ShaderManager {
	public:
		//enum ShaderType { Flat, FlatDirectional,DirectionalDiffuseMat, DirectionalDiffuseTex, DirectionalDiffuseArray, DirectionalDiffuseTexArray, MAX_SHADERS };
		static ShaderManager * Create(RenderDevice* pdevice);
		virtual ~ShaderManager() = default;
		virtual void* GetShaderDataByName(const char*name) = 0;
		//virtual void* GetShaderData(const char* shaderPath) = 0;
		virtual void* CreateShaderData(const char* shaderPath, bool cullBackFaces = true, bool enableBlend = true,  ShaderStorageType* ptypes = nullptr, uint32_t numtypes=0) = 0;
		//virtual void* GetShaderAttribute(ShaderAttrData&data) = 0;		
		
	};
}
