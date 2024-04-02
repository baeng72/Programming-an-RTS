#include "GLShader.h"

namespace GL {
	GLShader::GLShader(Renderer::RenderDevice* pdevice, void* shaderData)
		:_pdevice(pdevice),
		//_pshader(reinterpret_cast<ShaderUtil*>(shaderData)),
		_pshaderData(reinterpret_cast<GLShaderData*>(shaderData))
	{
		
		
		for (auto& pair : _pshaderData->resourceLocationMap) {
			_resourceKeys.push_back(pair.first);
			_resourceHandles[pair.first].push_back(0);
			_resourceType[pair.first] = -1;
		}
		for (auto& bindingset : _pshaderData->reflection.bindings) {
			
			for (auto& binding : bindingset) {
				size_t hash = Core::HashFNV1A(binding.name.c_str(), binding.name.length());
				switch (binding.descriptorType) {
				case SPIRVUtils::SPIRVDescriptorType::UniformBuffer:
					_resourceType[hash] = 0;
					break;
				case SPIRVUtils::SPIRVDescriptorType::StorageBuffer:
					_resourceType[hash] = 1;
					break;
				case SPIRVUtils::SPIRVDescriptorType::CombinedImageSampler:
					_resourceType[hash] = 2;
					break;
				default:
					assert(0);
					break;
				}
			}
		}
	}
	GLShader::~GLShader() {

	}
	void GLShader::Bind(uint32_t* pdynoffsets, uint32_t dynoffcount,bool override)
		
	{
		
		//_pshader->Bind();
		if (_pshaderData->enableDepth) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(_pshaderData->depthCompare);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
		GLERR();

		if (_pshaderData->enableCull) {
			glEnable(GL_CULL_FACE);
			glCullFace(_pshaderData->cullFace);
		}
		else {
			glDisable(GL_CULL_FACE);
		}
		GLERR();
		if (_pshaderData->enableBlend) {
			glEnable(GL_BLEND);
			glBlendEquation(_pshaderData->colorOp);
			glBlendFuncSeparate(_pshaderData->srcColor, _pshaderData->dstColor, _pshaderData->srcAlpha, _pshaderData->dstAlpha);
			
		}
		else {
			glDisable(GL_BLEND);
		}
		GLERR();
		GLint id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &id);
		if (id != _pshaderData->programID)
			glUseProgram(_pshaderData->programID);
		GLERR();
		
		//if (pdynoffsets && dynoffcount > 0) {

		//	/*GLint buffer=0;
		//	if (_storageBuffer != UINT32_MAX)
		//		buffer = _storageBuffer;
		//	else {
		//		glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &buffer);
		//		GLERR();
		//	}
		//	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, (GLintptr)pdynoffsets[0],20*64);
		//	GLERR();*/
		//}

		//bind resources
		uint32_t dynassigned = 0;
		for (auto& pair : _pshaderData->resourceLocationMap) {
			int type = _resourceType[pair.first];
			int binding = pair.second;
			auto& handles = _resourceHandles[pair.first];
			switch (type) {
			case 0://Uniform buffer;			
				glBindBufferBase(GL_UNIFORM_BUFFER, binding, handles[0]);
				break;
			case 1://Storage buffer
				
				if (pdynoffsets && dynassigned < dynoffcount) {
					glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding, handles[0], (GLintptr)pdynoffsets[dynassigned], 20 * 64);
					dynassigned++;//need to fix this mess
				}
				else {
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, handles[0]);
				}
				
				break;
			case 2://texture
				glBindTextures(binding, (GLsizei)handles.size(),(const GLuint*) handles.data());
				break;
			}
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
		GLuint buffer = *((GLuint*)pbuffer->GetNativeHandle());
		assert(i < _pshaderData->resourceLocationMap.size());
		size_t hash = _resourceKeys[i];
		_resourceHandles[hash] = { buffer };
		/*int binding = _pshaderData->resourceLocationMap.at(i);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer);
		GLERR();*/
		return true;
	}
	bool GLShader::SetUniformBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic)
	{
		GLuint buffer = *((GLuint*)pbuffer->GetNativeHandle());
		//_pshader->SetUniformBuffer(pname, buffer);		
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		assert(_pshaderData->resourceLocationMap.find(hash) != _pshaderData->resourceLocationMap.end());
		_resourceHandles[hash] = { buffer };
		/*int binding = _pshaderData->resourceLocationMap[hash];
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer);
		GLERR();*/
		return true;
	}
	bool GLShader::SetUniformData(uint32_t i, void* ptr, uint32_t len, bool dynamic)
	{
		ensureProgram();

		switch (len) {
			case 4:
				glUniform1f(i, *(float*)ptr);
			break;
			case 8:
				glUniform2fv(i, 1, (float*)ptr);
				break;
			case 12:
				glUniform3fv(i, 1, (float*)ptr);
				break;
			case 16:
				glUniform4fv(i, 1, (float*)ptr);
				break;
			case 64:
				glUniformMatrix4fv(i, 1, GL_FALSE, (float*)ptr);
				break;
			default:
				return false;
		}
		GLERR();
		return true;
	}
	bool GLShader::SetUniformData(const char* pname, void* ptr, uint32_t len, bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int i = _pshaderData->uniformLocationMap[hash];
		return SetUniformData(i, ptr, len, dynamic);
		/*switch (len) {
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
		return false;*/
	}
	bool GLShader::SetUniform(const char* pname, int32_t v)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform1i(location, v);
		//_pshader->setInt(pname, v);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, int32_t v)
	{
		//_pshader->setInt(i, v);
		ensureProgram();
		glUniform1i(i, v);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, float f)
	{
		//_pshader->setFloat(pname, f);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform1f(location, f);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, float f)
	{
		//_pshader->setFloat(i, f);
		ensureProgram();
		glUniform1f(i, f);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec2& v)
	{
		//_pshader->setVec2(i, &v);
		ensureProgram();
		glUniform2fv(i, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec2* p)
	{
		//_pshader->setVec2(i, p);
		ensureProgram();
		glUniform2fv(i, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec2& v)
	{
		//_pshader->setVec2(pname, &v);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform2fv(location, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec2* p)
	{
		//_pshader->setVec2(pname, p);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform2fv(location, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec3& v)
	{
		//_pshader->setVec3(i, &v);
		ensureProgram();
		glUniform3fv(i, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec3* p)
	{
		//_pshader->setVec3(i, p);
		ensureProgram();
		glUniform3fv(i, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec3& v)
	{
		//_pshader->setVec3(pname, &v);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform3fv(location, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec3* p)
	{
		//_pshader->setVec3(pname, p);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform3fv(location, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec4& v)
	{
		//_pshader->setVec4(i, &v);
		ensureProgram();
		glUniform4fv(i, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, vec4* p)
	{
		//_pshader->setVec4(i, p);
		ensureProgram();
		glUniform4fv(i, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec4& v)
	{
		//_pshader->setVec4(pname,&v);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform4fv(location, 1, &v[0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, vec4* p)
	{
		//_pshader->setVec4(pname, p);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniform4fv(location, 1, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, mat4& v)
	{
		//_pshader->setMat4(i, &v);
		ensureProgram();
		glUniformMatrix4fv(i, 1, GL_FALSE, &v[0][0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(uint32_t i, mat4* p)
	{
		//_pshader->setMat4(i, p);
		ensureProgram();
		glUniformMatrix4fv(i, 1, GL_FALSE, (const GLfloat*)p);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, mat4& v)
	{
		//_pshader->setMat4(pname, &v);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniformMatrix4fv(location, 1, GL_FALSE, &v[0][0]);
		GLERR();
		return true;
	}
	bool GLShader::SetUniform(const char* pname, mat4* p)
	{
		//_pshader->setMat4(pname, p);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)p);
		GLERR();
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
		//return _pshader->GetUniformLocation(pname);
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		return _pshaderData->uniformLocationMap[hash];
	}
	bool GLShader::SetTexture(uint32_t i, Renderer::Texture* ptexture) {
		
		GLTextureInfo* pinfo = (GLTextureInfo*)ptexture->GetNativeHandle();
		//_pshader->SetTexture(pinfo->textureID);
		size_t hash = _resourceKeys[i];
		_resourceHandles[hash] = { pinfo->textureID };
		ensureProgram();		
		glBindTextures(i, 1, &pinfo->textureID);
		GLERR();

		return true;
	}

	bool GLShader::SetTexture(const char* pname, Renderer::Texture* ptexture) {
		
		GLTextureInfo* pinfo = (GLTextureInfo*)ptexture->GetNativeHandle();
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _pshaderData->uniformLocationMap[hash];
		int offset = _pshaderData->texOffsetMap[hash];
		_resourceHandles[hash] = { pinfo->textureID };
		ensureProgram();
		/*glActiveTexture(GL_TEXTURE0 + location);
		GLERR();
		glBindTexture(pinfo->type, pinfo->textureID);	
		GLERR();
		glUniform1i(location,location);*/
		glBindTextures(location,1, &pinfo->textureID);
		GLERR();
		return true;
	}

	bool GLShader::SetTexture(uint32_t i, Renderer::Texture** pptexture, uint32_t count)
	{
		
		GLenum wrapMode;
		GLenum filter;
		std::vector<GLuint> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo*pinfo=(GLTextureInfo*)pptexture[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			wrapMode = pinfo->wrapMode;
			filter = pinfo->filter;
		}
		ensureProgram();
		glBindTextures(i, count, texids.data());
		//_resourceHandles[]
		GLERR();
		/*for (uint32_t i = 0; i < count; i++) {
			int texid = texids[i];			
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texid);
			int loc = _pshaderData->texSlots[i];
			glUniform1i(loc, i);
			GLERR();
		}
		GLERR();*/
		return true;
	}
	bool GLShader::SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count)
	{
		
		GLenum wrapMode;
		GLenum filter;
		std::vector<GLuint> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo* pinfo = (GLTextureInfo*)pptexture[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			wrapMode = pinfo->wrapMode;
			filter = pinfo->filter;
		}
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		assert(_pshaderData->uniformLocationMap.find(hash) != _pshaderData->uniformLocationMap.end());
		int location = _pshaderData->uniformLocationMap[hash];
		ensureProgram();
		glBindTextures(location, count, texids.data());
		_resourceHandles[hash] = texids;
		GLERR();
		/*std::vector<int> locs(count);
		int offset = _pshaderData->texOffsetMap[hash];
		for (uint32_t i = 0; i < count; i++) {
			glActiveTexture(GL_TEXTURE0 + offset + i);
			glBindTexture(GL_TEXTURE_2D, texids[i]);
			locs[i] = offset + i;
		}
		glUniform1iv(location, count, (const GLint*)locs.data());
		GLERR();*/
		
		return true;
	}
	bool GLShader::SetTextures(Renderer::Texture** pptextures, uint32_t count)
	{
		
		GLenum wrapMode;
		GLenum filter;
		std::vector<GLuint> texids(count);
		for (uint32_t i = 0; i < count; i++) {
			GLTextureInfo* pinfo = (GLTextureInfo*)pptextures[i]->GetNativeHandle();
			texids[i] = pinfo->textureID;
			wrapMode = pinfo->wrapMode;
			filter = pinfo->filter;
		}
		ensureProgram();
		glBindTextures(0, count, texids.data());
		
		for (auto& pair : _pshaderData->texOffsetMap) {
			_resourceHandles[pair.first] = { texids[pair.second] };
		}
		GLERR();
		/*for (uint32_t i = 0; i < count; i++) {
			int texid = texids[i];
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texid);
			int loc = _pshaderData->texSlots[i];
			glUniform1i(loc, i);
			GLERR();
		}
		*/
		return true;
	}
	uint32_t GLShader::GetTextureId(const char* pname)
	{
		return uint32_t();
	}
	bool GLShader::SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic)
	{
		
		GLuint buffer = *(GLuint*)pbuffer->GetNativeHandle();
		assert(i < _pshaderData->resourceLocationMap.size());
		int binding = _pshaderData->resourceLocationMap.at(i);
		//_resourceBufferMap.at(i) = buffer;
		size_t hash = _resourceKeys[i];
		_resourceHandles[hash] = { buffer };
		/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer);
		GLERR();*/
		//_pshader->SetStorageBuffer(i,buffer);		
		return true;
	}
	bool GLShader::SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic)
	{
		GLuint buffer = *(GLuint*)pbuffer->GetNativeHandle();
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		assert(_pshaderData->resourceLocationMap.find(hash) != _pshaderData->resourceLocationMap.end());
		//_resourceBufferMap[hash] = buffer;
		_resourceHandles[hash] = { buffer };
		//int binding = _pshaderData->resourceLocationMap[hash];
		////_storageBuffer = buffer;
		////_pshader->SetStorageBuffer(pname,buffer);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer);
		//GLERR();
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
		return (void*)_pshaderData->programID;// _pshader->getProgramID();
	}

	void GLShader::ensureProgram() {
		GLint id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &id);
		if (id != _pshaderData->programID)
			glUseProgram(_pshaderData->programID);
	}
}