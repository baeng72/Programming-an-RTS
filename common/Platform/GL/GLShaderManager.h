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
	public:		
		GLShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~GLShaderManager();
		virtual void* GetShaderDataByName(const char* name)override;		
		virtual void* CreateShaderData(const char* shaderPath, bool cullBackFaces = true, bool enableBlend = true, bool enableDepth = true, Renderer::ShaderStorageType* ptypes = nullptr, uint32_t numtypes = 0)override;
	};
}