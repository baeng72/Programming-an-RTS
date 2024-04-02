#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "../../Renderer/ShaderManager.h"
#include "ShaderUtil.h"
#include <glad/glad.h>
#include "../SPIRV/SPIRVCompiler.h"
#include "../SPIRV/SPIRVReflector.h"
namespace GL {
	struct GLShaderData {
		SPIRVUtils::SPIRVShaderReflection reflection;
		std::unordered_map<size_t, int> resourceLocationMap;		
		std::unordered_map<size_t, int> uniformLocationMap;		
		std::unordered_map<size_t, int> texOffsetMap;
		std::vector<GLint> texSlots;
		int programID;
		GLenum frontFace;		
		bool enableCull;
		GLenum cullFace;
		bool enableDepth;
		GLenum depthCompare;
		bool enableBlend;
		GLenum srcColor;
		GLenum dstColor;
		GLenum colorOp;
		GLenum srcAlpha;
		GLenum dstAlpha;
		GLenum alphaOp;
	};

	class GLShaderManager : public Renderer::ShaderManager {
		Renderer::RenderDevice* _pdevice;
		std::unordered_map<std::string, std::unique_ptr<ShaderUtil>> _shaderList;
		std::unordered_map<std::string, std::unique_ptr<GLShaderData>> _shaderList2;
		std::string _path;
		std::string readFile(const std::string& filepath);
		SPIRVUtils::SPIRVShaderStage ShaderTypeFromString(const std::string& type);
		std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string> PreProcess(std::string& src);
		void CompileShaderFile(const std::string& name, std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string>& shaderSources, Renderer::ShaderCreateInfo& createInfo);
		GLenum GetBlendFactor(Renderer::ShaderBlendFactor factor);
		GLenum GetBlendOp(Renderer::ShaderBlendOp op);
		GLenum GetDepthCompare(Renderer::ShaderCompareOp op);
		GLenum getShaderType(SPIRVUtils::SPIRVShaderStage stage);
		GLuint compileShader(const char* shaderSrc, GLenum shaderType);
	public:		
		GLShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~GLShaderManager();
		virtual void* GetShaderDataByName(const char* name)override;		
		virtual void* CreateShaderData(const char* shaderPath, Renderer::ShaderCullMode cullMode=Renderer::ShaderCullMode::backFace, bool enableBlend = true, bool enableDepth = true, Renderer::ShaderStorageType* ptypes = nullptr, uint32_t numtypes = 0,void *platformData=nullptr)override;
		virtual void* CreateShaderData(const char* shaderPath, Renderer::ShaderCreateInfo& createInfo) override;
		virtual void* CreateShaderData(const char* name, const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, Renderer::ShaderCreateInfo& info)override;
	};
}