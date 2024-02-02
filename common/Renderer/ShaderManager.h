#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Light.h"
#include "Texture.h"
#include "Buffer.h"
namespace Renderer {
	enum class ShaderStorageType { Uniform, UniformDynamic, Storage, StorageDynamic, Texture, TextureArray };
	enum class ShaderCullMode{frontFace,backFace,frontandbackFace, None	};
	enum class ShaderBlendFactor{Zero,One,SrcColor,OneMinusSrcColor,DstColor,OneMinusDstColor,SrcAlpha,OneMinusSrcAlpha,DstAlpha,OneMinusDstAlpha,ConstantColor,OneMinusConstantColor,ConstantAlpha,OneMinusConstantAlpha};
	enum class ShaderBlendOp{Add,Subtract,ReverseSubstract,Min,Max};
	enum class ShaderCompareOp{Never,Less,Equal,LessOrEqual,Greater,NotEqual,GreaterOrEqual,Always};
	enum class ShaderTopologyType{PointList,LineList,TriangleList};
	//enum ShaderAttrFlagBits {
	//	SHADER_ATTR_UBO	= 0x0000001,		//uniform buffer
	//	SHADER_ATTR_STORAGE	= 0x000002,		//storage buffer
	//	SHADER_ATTR_SAMPLER	= 0x000004,		//image sampler
	//	SHADER_ATTR_SAMPLER_ARRAY = 0x000008,//array of sampler
	//};
	//using ShaderAttrFlags = uint32_t;
	//enum ShaderStageFlagBits {
	//	SHADER_STAGE_VERTEX=0x000001,
	//	SHADER_STAGE_GEOMETRY=0x00002,
	//	SHADER_STAGE_FRAGMENT=0x00004,
	//};
	//
	//using ShaderStageFlags = uint32_t;
	//struct ShaderAttrData {
	//	const char* name;
	//	uint32_t flags;
	//	uint32_t stages;
	//	uint32_t size;
	//	uint32_t count;
	//};
	
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
	struct ShaderBlendInfo {
		bool enable{ false };
		ShaderBlendFactor srcColorFactor{ ShaderBlendFactor::SrcColor };
		ShaderBlendFactor dstColorFactor{ ShaderBlendFactor::Zero };
		ShaderBlendOp colorOp{ ShaderBlendOp::Add };
		ShaderBlendFactor srcAlphaFactor{ ShaderBlendFactor::SrcAlpha };
		ShaderBlendFactor dstAlphaFactor{ ShaderBlendFactor::Zero };
		ShaderBlendOp alphaOp{ ShaderBlendOp::Add };
	};
	struct ShaderDepthInfo {
		bool enable{ false };
		ShaderCompareOp compare{ ShaderCompareOp::LessOrEqual };
	};
	struct ShaderCreateInfo {
		ShaderCullMode cullMode{ ShaderCullMode::backFace };
		ShaderBlendInfo blendInfo;
		ShaderDepthInfo depthInfo;
		ShaderTopologyType topologyType{ ShaderTopologyType::TriangleList };
		ShaderStorageType* ptypes{ nullptr };
		uint32_t numtypes{ 0 };
		void* platformData{ nullptr };
	};
	class ShaderManager {
	public:
		//enum ShaderType { Flat, FlatDirectional,DirectionalDiffuseMat, DirectionalDiffuseTex, DirectionalDiffuseArray, DirectionalDiffuseTexArray, MAX_SHADERS };
		static ShaderManager * Create(RenderDevice* pdevice);
		
		virtual ~ShaderManager() = default;
		virtual void* GetShaderDataByName(const char*name) = 0;
		//virtual void* GetShaderData(const char* shaderPath) = 0;
		virtual void* CreateShaderData(const char* shaderPath, ShaderCullMode mode=ShaderCullMode::backFace, bool enableBlend = true,bool enableDepth=true,  ShaderStorageType* ptypes = nullptr, uint32_t numtypes=0,  void* platformData = nullptr) = 0;
		virtual void* CreateShaderData(const char* shaderPath, ShaderCreateInfo& createInfo)=0;
		virtual void* CreateShaderData(const char* name, const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, ShaderCreateInfo& info) = 0;
		//virtual void* GetShaderAttribute(ShaderAttrData&data) = 0;		
		
	};
}
