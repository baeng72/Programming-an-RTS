#pragma once
#include "../../Renderer/Buffer.h"
#include <glad/glad.h>
namespace GL {
	class GLBuffer : public Renderer::Buffer {
		Renderer::RenderDevice* _pdevice;
		uint32_t _size;
		GLuint _buffer;
		bool _isDynamic;
		bool _isUniform;
		bool _mapped;
	public:
		GLBuffer(Renderer::RenderDevice* pdevice, void* ptr, uint32_t size, uint32_t count, bool isUniform, bool isDynamic);
		virtual ~GLBuffer();
		virtual void Set(void* ptr, uint32_t size, uint32_t offset)override;
		virtual void* GetNativeHandle()const override;
		virtual void* MapPtr() override;
		virtual void UnmapPtr() override;
		virtual uint32_t GetSize()const override;
		virtual void SetName(const char* pname)override {}
	};
}