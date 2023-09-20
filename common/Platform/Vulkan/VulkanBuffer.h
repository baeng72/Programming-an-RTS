#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Buffer.h"
#include "VulkanEx.h"
namespace Vulkan {
	struct VulkanBufferData {
		Vulkan::Buffer buffer;
		void* ptr;
		uint32_t size;
		VkDescriptorType descriptorType;
	};
	class VulkanBufferImpl : public Renderer::Buffer {
		Renderer::RenderDevice* _pdevice;
		/*Vulkan::Buffer _buffer;
		void* _ptr;
		uint32_t _size;*/
		VulkanBufferData _data;
	public:
		VulkanBufferImpl (Renderer::RenderDevice* pdevice, void* ptr, uint32_t size,uint32_t count,bool isUniform,bool isDynamic);
		virtual ~VulkanBufferImpl();
		virtual void Set(void* ptr, uint32_t size,uint32_t offset)override;
		virtual void* GetNativeHandle()const override;
		virtual void* GetPtr()const override;
		virtual uint32_t GetSize()const override;
	};
}