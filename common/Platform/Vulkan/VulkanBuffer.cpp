
#include "VulkanBuffer.h"
#include "VulkState.h"

namespace Vulkan {
	
	
	
	VulkanBufferImpl::VulkanBufferImpl(Renderer::RenderDevice* pdevice, void* ptr, uint32_t size,uint32_t count,bool isUniform,bool isDynamic):_pdevice(pdevice)
	{
		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		if (isDynamic) {
			if (isUniform)
				descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			else
				descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		}
		else {
			if (isUniform)
				descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			else
				descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
			
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		std::vector<UniformBufferInfo> bufferInfo;
		UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, descriptorType,true)
			.AddBuffer((VkDeviceSize)size, count, 1)
			.build(_data.buffer,bufferInfo);
		_data.ptr = bufferInfo[0].ptr;
		_data.size = size;
		if (ptr)
			memcpy(_data.ptr, ptr, size);
		_data.descriptorType = descriptorType;
	}

	VulkanBufferImpl::~VulkanBufferImpl()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::cleanupBuffer(context.device, _data.buffer);
	}

	void VulkanBufferImpl::Set(void* ptr, uint32_t size,uint32_t offset)
	{
		void* dstptr = (void*)((uint8_t*)_data.ptr + offset);
		memcpy(dstptr, ptr, size);
	}

	void* VulkanBufferImpl::GetNativeHandle() const
	{
		return (void*)&_data;
	}

	void VulkanBufferImpl::UnmapPtr() 
	{
	}

	uint32_t VulkanBufferImpl::GetSize() const
	{
		return _data.size;
	}

	void* VulkanBufferImpl::MapPtr() {
		return _data.ptr;
	}

}
