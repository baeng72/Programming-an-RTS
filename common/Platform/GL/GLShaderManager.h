#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "../../Renderer/ShaderManager.h"
#include "ShaderUtil.h"
#include <glad/glad.h>

namespace GL {
	class GLShaderManager : public Renderer::ShaderManager {
		Renderer::RenderDevice* _pdevice;
		std::unordered_map<std::string, std::unique_ptr<ShaderUtil>> _shaderList;
		GLenum GetBlendFactor(Renderer::ShaderBlendFactor factor);
		GLenum GetBlendOp(Renderer::ShaderBlendOp op);
	public:		
		GLShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~GLShaderManager();
		virtual void* GetShaderDataByName(const char* name)override;		
		virtual void* CreateShaderData(const char* shaderPath, Renderer::ShaderCullMode cullMode=Renderer::ShaderCullMode::backFace, bool enableBlend = true, bool enableDepth = true, Renderer::ShaderStorageType* ptypes = nullptr, uint32_t numtypes = 0,void *platformData=nullptr)override;
		virtual void* CreateShaderData(const char* shaderPath, Renderer::ShaderCreateInfo& createInfo) override;
		virtual void* CreateShaderData(const char* name, const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, Renderer::ShaderCreateInfo& info)override;
	};
}