
#include "../../Core/Log.h"
#include "VulkanTexture.h"
#include "VulkState.h"
#include <Windows.h>

namespace Vulkan {
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size):_size(size)
	{		
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		std::vector<Vulkan::Texture> textures;
		
		TextureLoader::begin(context.device, context.commandBuffer, context.queue, context.memoryProperties)
			.addTexture(pfile)
			.load(textures);
		_texture = textures[0];
		if (_size.x == -1 || _size.y == -1) {
			_size = glm::vec2(_texture.width, _texture.height);
		}
	}
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, int bytesperpixel, uint8_t* pixels):_size(glm::vec2(width,height))
	{
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		bool enableLod = false;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		switch (bytesperpixel) {
		case 1:
			format = VK_FORMAT_R8_UNORM;
			break;
		case 2:
			format = VK_FORMAT_R8G8_UNORM;
			break;
		case 3:
			format = VK_FORMAT_R8G8B8_UNORM;
			break;		
		case 4:
			format = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}
		ASSERT(format != VK_FORMAT_MAX_ENUM, "Unkown format for bytesperpixel: {0}", bytesperpixel);
		VkDeviceSize imageSize = (uint64_t)width * (uint64_t)height * bytesperpixel;
		TextureProperties props;
		props.format = format;
		props.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		props.height = height;
		props.width = width;
		props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		props.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		props.mipLevels = enableLod ? 0 : 1;
#ifdef __USE__VMA__
		props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
		props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
		initTexture(context.device, context.memoryProperties, props, _texture);
		transitionImage(context.device, context.queue, context.commandBuffer, _texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _texture.mipLevels);

		Buffer stagingBuffer;
		BufferProperties bufProps;
#ifdef __USE__VMA__
		bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
		bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
#endif
		bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufProps.size = imageSize;
		initBuffer(context.device, context.memoryProperties, bufProps, stagingBuffer);
		void* ptr = mapBuffer(context.device, stagingBuffer);
		memcpy(ptr, pixels, imageSize);
		CopyBufferToImage(context.device, context.queue, context.commandBuffer, stagingBuffer, _texture, width, height);
		generateMipMaps(context.device, context.queue, context.commandBuffer, _texture);
		unmapBuffer(context.device, stagingBuffer);
		cleanupBuffer(context.device, stagingBuffer);

		
	}
	VulkanTextureImpl::~VulkanTextureImpl()
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		vkDeviceWaitIdle(context.device);
		Vulkan::cleanupTexture(context.device, _texture);
	}
	
	void* VulkanTextureImpl::GetNativeHandle() const
	{
		return (void*)&_texture;
	}

	glm::vec2 VulkanTextureImpl::GetScale()const {
		glm::vec2 scale = glm::vec2(_size.x/(float)_texture.width,_size.y/(float) _texture.height);
		return scale;
	}
}
