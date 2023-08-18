#pragma once
#include "RenderDevice.h"
namespace Renderer {
	class Buffer {
	public:
		static Buffer* Create(Renderer::RenderDevice* pdevice, uint32_t size,bool isUniform=false,bool isDynamic=false);
		static Buffer* Create(Renderer::RenderDevice* pdevice, void*ptr,uint32_t size,bool isUniform=false,bool isDynamic=false);		
		virtual ~Buffer() = default;	
		virtual void Set(void* ptr, uint32_t size)=0;
		virtual void* GetNativeHandle()const = 0;
		virtual uint32_t GetSize()const = 0;
	};
}