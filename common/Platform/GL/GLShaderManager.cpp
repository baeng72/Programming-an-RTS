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
	void* GLShaderManager::CreateShaderData(const char* shaderPath, bool cullBackFaces, bool enableBlend, Renderer::ShaderStorageType* ptypes, uint32_t numtypes)
	{
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(shaderPath);
			_shaderList[name] = std::move(shader);
		}
		return _shaderList[name].get();
	}
}