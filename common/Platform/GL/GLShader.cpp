#include "GLShader.h"

namespace GL {
	GLShader::GLShader(Renderer::RenderDevice* pdevice, void* shaderData)
		:_pdevice(pdevice),_pshader(reinterpret_cast<ShaderUtil*>(shaderData))
	{
	}
	GLShader::~GLShader() {

	}
	void GLShader::Bind(uint32_t* pdynoffsets, uint32_t dynoffcount)
	{
		_pshader->Bind();
	}
	void GLShader::SetWireframe(bool wireframe)
	{
	}
	void GLShader::SetPushConstData(void*, uint32_t len)
	{
	}
	bool GLShader::SetUniformBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic)
	{
		
		return false;
	}
	bool GLShader::SetUniformBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic)
	{
		
		return false;
	}
	bool GLShader::SetUniformData(uint32_t i, void* ptr, uint32_t len, bool dynamic)
	{
		switch (len) {
			case 4:
				_pshader->setFloat(i, *(float*)ptr);
				return true;
			break;
			case 8:
				_pshader->setVec2(i, *(vec2*)ptr);
				return true;
				break;
			case 12:
				_pshader->setVec3(i, *(vec3*)ptr);
				return true;
				break;
			case 16:
				_pshader->setVec4(i, *(vec4*)ptr);
				return true;
				break;
			case 64:
				_pshader->setMat4(i, *(mat4*)ptr);
				return true;
				break;

		}
		return false;
	}
	bool GLShader::SetUniformData(const char* pname, void* ptr, uint32_t len, bool dynamic)
	{
		int i = _pshader->GetUniformLocation(pname);
		switch (len) {
		case 4:
			_pshader->setFloat(i, *(float*)ptr);
			return true;
			break;
		case 8:
			_pshader->setVec2(i, *(vec2*)ptr);
			return true;
			break;
		case 12:
			_pshader->setVec3(i, *(vec3*)ptr);
			return true;
			break;
		case 16:
			_pshader->setVec4(i, *(vec4*)ptr);
			return true;
			break;
		case 64:
			_pshader->setMat4(i, *(mat4*)ptr);
			return true;
			break;

		}
		return false;
	}
	uint32_t GLShader::GetUniformId(const char* pname, bool dynamic)
	{
		return _pshader->GetUniformLocation(pname);
	}
	bool GLShader::SetTexture(uint32_t, Renderer::Texture** pptexture, uint32_t count)
	{
		return false;
	}
	bool GLShader::SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count)
	{
		return false;
	}
	bool GLShader::SetTextures(Renderer::Texture** pptextures, uint32_t count)
	{
		return false;
	}
	uint32_t GLShader::GetTextureId(const char* pname)
	{
		return uint32_t();
	}
	bool GLShader::SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic)
	{
		return false;
	}
	bool GLShader::SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic)
	{
		return false;
	}
	bool GLShader::SetStorageData(uint32_t i, void* ptr, uint32_t len, bool dynamic)
	{
		return false;
	}
	bool GLShader::SetStorageData(const char* pname, void* ptr, uint32_t len, bool dynamic)
	{
		return false;
	}
	uint32_t GLShader::GetStorageId(const char* pname, bool dynamic)
	{
		return uint32_t();
	}
}