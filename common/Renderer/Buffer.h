#pragma once
#include "RenderDevice.h"
namespace Renderer {
	class Buffer {
	public:
		static Buffer* Create(Renderer::RenderDevice* pdevice, uint32_t size,uint32_t count=1,bool isUniform=false,bool isDynamic=false);
		static Buffer* Create(Renderer::RenderDevice* pdevice, void*ptr,uint32_t size,uint32_t count=1,bool isUniform=false,bool isDynamic=false);		
		virtual ~Buffer() = default;	
		virtual void Set(void* ptr, uint32_t size,uint32_t offset=0)=0;
		
		virtual void* GetNativeHandle()const = 0;
		virtual void* MapPtr() = 0;
		virtual void  UnmapPtr() = 0;
		virtual uint32_t GetSize()const = 0;
		virtual void SetName(const char* pname) = 0;
	};
}