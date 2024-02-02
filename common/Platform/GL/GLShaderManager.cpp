#include "GLShaderManager.h"
#include "../../Core/Log.h"
namespace GL {
	GLShaderManager::GLShaderManager(Renderer::RenderDevice* pdevice)
		:_pdevice(pdevice)
	{
	}
	GLShaderManager::~GLShaderManager()
	{
		_shaderList.clear();
	}
	void* GLShaderManager::GetShaderDataByName(const char* name)
	{
		ASSERT(_shaderList.find(name) != _shaderList.end(), "Unknown shader requested!");
		return _shaderList[name].get();
	}
	void* GLShaderManager::CreateShaderData(const char* shaderPath, Renderer::ShaderCullMode cullMode, bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes, void* platformData)
	{
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(shaderPath);
			//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);

			shader->EnableCull(cullMode != Renderer::ShaderCullMode::None);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
			if (cullMode == Renderer::ShaderCullMode::frontFace)
				shader->SetCullFace(GL_BACK);
			else if (cullMode == Renderer::ShaderCullMode::backFace)
				shader->SetCullFace(GL_FRONT);
#else
			if (cullMode == Renderer::ShaderCullMode::frontFace)
				shader->SetCullFace(GL_FRONT);
			else if (cullMode == Renderer::ShaderCullMode::backFace)
				shader->SetCullFace(GL_BACK);
#endif
			else if (cullMode == Renderer::ShaderCullMode::frontandbackFace)
				shader->SetCullFace(GL_FRONT_AND_BACK);
			shader->EnableBlend(enableBlend);
			shader->EnableDepth(enableDepth);
			_shaderList[name] = std::move(shader);
		}
		return _shaderList[name].get();
	}
	GLenum GLShaderManager::GetBlendFactor(Renderer::ShaderBlendFactor factor) {
		GLenum res = 0;
		switch (factor) {
		case Renderer::ShaderBlendFactor::ConstantAlpha:
			res = GL_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::ConstantColor:
			res = GL_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::DstAlpha:
			res = GL_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::DstColor:
			res = GL_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::One:
			res = GL_ONE;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantAlpha:
			res = GL_ONE_MINUS_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantColor:
			res = GL_ONE_MINUS_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstAlpha:
			res = GL_ONE_MINUS_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstColor:
			res = GL_ONE_MINUS_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcAlpha:
			res = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcColor:
			res = GL_ONE_MINUS_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::SrcAlpha:
			res = GL_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::SrcColor:
			res = GL_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::Zero:
			res = GL_ZERO;
			break;
		}
		return res;
	}
	GLenum GLShaderManager::GetBlendOp(Renderer::ShaderBlendOp op) {
		GLenum res = 0;
		switch (op) {
		case Renderer::ShaderBlendOp::Add:
			res = GL_FUNC_ADD;
			break;
		case Renderer::ShaderBlendOp::Max:
			res = GL_MAX;
			break;
		case Renderer::ShaderBlendOp::Min:
			res = GL_MIN;
			break;
		case Renderer::ShaderBlendOp::ReverseSubstract:
			res = GL_FUNC_REVERSE_SUBTRACT;
			assert(0);
			break;
		case Renderer::ShaderBlendOp::Subtract:
			res = GL_FUNC_SUBTRACT;
			break;
		}
		return res;
	}
	void* GLShaderManager::CreateShaderData(const char* shaderPath, Renderer::ShaderCreateInfo&info)
	{
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(shaderPath);
			//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);

			shader->EnableCull(info.cullMode != Renderer::ShaderCullMode::None);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
			if (info.cullMode == Renderer::ShaderCullMode::frontFace)
				shader->SetCullFace(GL_BACK);
			else if (info.cullMode == Renderer::ShaderCullMode::backFace)
				shader->SetCullFace(GL_FRONT);
#else
			if (info.cullMode == Renderer::ShaderCullMode::frontFace)
				shader->SetCullFace(GL_FRONT);
			else if (info.cullMode == Renderer::ShaderCullMode::backFace)
				shader->SetCullFace(GL_BACK);
#endif

			else if (info.cullMode == Renderer::ShaderCullMode::frontandbackFace)
				shader->SetCullFace(GL_FRONT_AND_BACK);
			shader->EnableBlend(info.blendInfo.enable);
			if (info.blendInfo.enable) {
				shader->SetBlendFunction(GetBlendFactor(info.blendInfo.srcColorFactor), GetBlendFactor(info.blendInfo.dstColorFactor), GetBlendOp(info.blendInfo.colorOp), GetBlendFactor(info.blendInfo.srcAlphaFactor), GetBlendFactor(info.blendInfo.dstAlphaFactor), GetBlendOp(info.blendInfo.alphaOp));
			}
			shader->EnableDepth(info.depthInfo.enable);
			
			_shaderList[name] = std::move(shader);
		}
		return _shaderList[name].get();
	}

	void* GLShaderManager::CreateShaderData(const char* name, const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, Renderer::ShaderCreateInfo& info) {
		if (_shaderList.find(name) != _shaderList.end()) {
			_shaderList[name].reset();
		}
		std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(vertexSrc,geometrySrc,fragmentSrc);
		//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);

		shader->EnableCull(info.cullMode != Renderer::ShaderCullMode::None);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
		if (info.cullMode == Renderer::ShaderCullMode::frontFace)
			shader->SetCullFace(GL_BACK);
		else if (info.cullMode == Renderer::ShaderCullMode::backFace)
			shader->SetCullFace(GL_FRONT);
#else
		if (info.cullMode == Renderer::ShaderCullMode::frontFace)
			shader->SetCullFace(GL_FRONT);
		else if (info.cullMode == Renderer::ShaderCullMode::backFace)
			shader->SetCullFace(GL_BACK);
#endif

		else if (info.cullMode == Renderer::ShaderCullMode::frontandbackFace)
			shader->SetCullFace(GL_FRONT_AND_BACK);
		shader->EnableBlend(info.blendInfo.enable);
		if (info.blendInfo.enable) {
			shader->SetBlendFunction(GetBlendFactor(info.blendInfo.srcColorFactor), GetBlendFactor(info.blendInfo.dstColorFactor), GetBlendOp(info.blendInfo.colorOp), GetBlendFactor(info.blendInfo.srcAlphaFactor), GetBlendFactor(info.blendInfo.dstAlphaFactor), GetBlendOp(info.blendInfo.alphaOp));
		}
		shader->EnableDepth(info.depthInfo.enable);
		
		_shaderList[name] = std::move(shader);

		return &_shaderList[name];
		
	}
}