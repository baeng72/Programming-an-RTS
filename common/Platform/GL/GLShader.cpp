#include "GLShader.h"

namespace GL {
	GLShader::GLShader(Renderer::RenderDevice* pdevice, void* shaderData)
		:_pdevice(pdevice),_pshader(reinterpret_cast<ShaderUtil*>(shaderData)),_buffer(UINT32_MAX)
	{
	}
	GLShader::~GLShader() {

	}
	void GLShader::Bind(uint32_t* pdynoffsets, uint32_t dynoffcount)
		
	{
		
		_pshader->Bind();
		if (pdynoffsets && dynoffcount > 0) {

			GLint buffer=0;
			if (_buffer != UINT32_MAX)
				buffer = _buffer;
			else {
				glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &buffer);
				GLERR();
			}
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, (GLintptr)pdynoffsets[0],20*64);
			GLERR();
		}
	}
	void GLShader::SetWireframe(bool wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}
	/*void GLShader::SetPushConstData(void*, uint32_t len)
	{
	}*/
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
		
		switch (len) {
		case 4:
			_pshader->setFloat(pname, (*(float*)ptr));
			return true;
			
		case 8:
			_pshader->setVec2(pname, (vec2*)ptr);
			return true;
			
		case 12:
			_pshader->setVec3(pname, (vec3*)ptr);
			return true;
			
		case 16:
			_pshader->setVec4(pname, (vec4*)ptr);
			return true;
			
		case 64:
			_pshader->setMat4(pname, (mat4*)ptr);
			return true;
		default:
			assert(0);
			break;

		}
		return false;
	}
	bool GLShader::SetUniform(uint32_t i, vec2& v)
	{
		_pshader->setVec2(i, &v);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec2* p)
	{
		_pshader->setVec2(i, p);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec2& v)
	{
		_pshader->setVec2(pname, &v);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec2* p)
	{
		_pshader->setVec2(pname, p);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec3& v)
	{
		_pshader->setVec3(i, &v);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec3* p)
	{
		_pshader->setVec3(i, p);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec3& v)
	{
		_pshader->setVec3(pname, &v);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec3* p)
	{
		_pshader->setVec3(pname, p);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec4& v)
	{
		_pshader->setVec4(i, &v);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec4* p)
	{
		_pshader->setVec4(i, p);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec4& v)
	{
		_pshader->setVec4(pname,&v);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec4* p)
	{
		_pshader->setVec4(pname, p);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, mat4& v)
	{
		_pshader->setMat4(i, &v);
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, mat4* p)
	{
		_pshader->setMat4(i, p);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, mat4& v)
	{
		_pshader->setMat4(pname, &v);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, mat4* p)
	{
		_pshader->setMat4(pname, p);
		return true;
	}
	bool GLShader::SetUniform(const char* pname, void* ptr, uint32_t len) {
		return SetUniformData(pname, ptr, len);
	}
	bool GLShader::SetUniform(uint32_t i, void* ptr, uint32_t size)
	{
		return SetUniformData(i, ptr, size);
	}
	uint32_t GLShader::GetUniformId(const char* pname, bool dynamic)
	{
		return _pshader->GetUniformLocation(pname);
	}
	bool GLShader::SetTexture(uint32_t, Renderer::Texture** pptexture, uint32_t count)
	{
		struct GLTextureInfo {
			int textureID;
			int width;
			int height;
			GLenum addrMode;
			GLenum filter;
		};
		GLenum addrMode;
		GLenum filter;
		std::vector<int> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo*pinfo=(GLTextureInfo*)pptexture[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			addrMode = pinfo->addrMode;
			filter = pinfo->filter;
		}
		_pshader->SetTextures(texids.data(),count);
		_pshader->SetSampler(addrMode, filter);
		return true;
	}
	bool GLShader::SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count)
	{
		struct GLTextureInfo {
			int textureID;
			int width;
			int height;
			GLenum addrMode;
			GLenum filter;
		};
		GLenum addrMode;
		GLenum filter;
		std::vector<int> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo* pinfo = (GLTextureInfo*)pptexture[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			addrMode = pinfo->addrMode;
			filter = pinfo->filter;
		}
		_pshader->SetTextures(pname, texids.data(), count);
		_pshader->SetSampler(addrMode, filter);
		return true;
	}
	bool GLShader::SetTextures(Renderer::Texture** pptextures, uint32_t count)
	{
		struct GLTextureInfo {
			int textureID;
			int width;
			int height;
			GLenum addrMode;
			GLenum filter;
		};
		GLenum addrMode;
		GLenum filter;
		std::vector<int> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo* pinfo = (GLTextureInfo*)pptextures[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			addrMode = pinfo->addrMode;
			filter = pinfo->filter;
		}
		_pshader->SetTextures(texids.data(), count);
		_pshader->SetSampler(addrMode, filter);
		return true;
	}
	uint32_t GLShader::GetTextureId(const char* pname)
	{
		return uint32_t();
	}
	bool GLShader::SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic)
	{
		
		GLuint buffer = *(GLuint*)pbuffer->GetNativeHandle();
		_pshader->SetStorageBuffer(buffer);
		
		return true;
	}
	bool GLShader::SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic)
	{
		GLuint buffer = *(GLuint*)pbuffer->GetNativeHandle();
		_buffer = buffer;
		_pshader->SetStorageBuffer(buffer);
		return true;
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
	void* GLShader::GetNativeHandle()
	{
		return _pshader->getProgramID();
	}
}