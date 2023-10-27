#include "GLBuffer.h"
#include "../../Core/Log.h"
#include "GLERR.h"
namespace GL {
	GLBuffer::GLBuffer(Renderer::RenderDevice* pdevice, void* ptr, uint32_t size, uint32_t count, bool isUniform, bool isDynamic)
		:_pdevice(pdevice)
	{
		_isDynamic = isDynamic;
		_isUniform=isUniform;
		_mapped = false;
		glGenBuffers(1, &_buffer);		
		if (isUniform) {
			glBindBuffer(GL_UNIFORM_BUFFER, _buffer);
			glBufferData(GL_UNIFORM_BUFFER, size*count, ptr, GL_DYNAMIC_DRAW);
			//glBindBufferBase(GL_UNIFORM_BUFFER, 0, _buffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size*count, ptr, GL_DYNAMIC_DRAW);
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);			
		}		
		_size = size*count;
	}
	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &_buffer);
	}
	void GLBuffer::Set(void* ptr, uint32_t size, uint32_t offset)
	{
		if (_mapped)
			UnmapPtr();
		if (_isUniform) {
			glBindBuffer(GL_UNIFORM_BUFFER, _buffer);
			glBufferSubData(GL_UNIFORM_BUFFER, offset, size, ptr);
		}
		else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, ptr);
		}
	}
	void* GLBuffer::GetNativeHandle() const
	{
		
		return (void*) &_buffer;
	}
	void* GLBuffer::MapPtr() 
	{
		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		GLenum bufferType = _isUniform ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
		
		glBindBuffer(bufferType, _buffer);
		
		void* ptr = glMapBufferRange(bufferType, 0, _size, bufMask);
		_mapped = true;
		return ptr;
	}
	void GLBuffer::UnmapPtr() 
	{
		GLenum bufferType = _isUniform ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
		glUnmapBuffer(bufferType);
		
		_mapped = false;
	}
	uint32_t GLBuffer::GetSize() const
	{
		return _size;
	}
}