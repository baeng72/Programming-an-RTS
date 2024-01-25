
#include "../../Core/Log.h"
#include "VulkanTexture.h"
#include "VulkState.h"
#include <Windows.h>
#include "stb/stb_image_write.h"


namespace Vulkan {
	VkSamplerAddressMode VulkanTextureImpl::GetSamplerAddrMode(Renderer::TextureSamplerAddress add)
	{
		VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		switch (add) {
		case Renderer::TextureSamplerAddress::Clamp:
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case Renderer::TextureSamplerAddress::Repeat:
			mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case Renderer::TextureSamplerAddress::Border:
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;		
		}
		return mode;
	}
	VkFilter VulkanTextureImpl::GetSamplerFilter(Renderer::TextureSamplerFilter filt) {
		VkFilter filter = VK_FILTER_LINEAR;
		switch (filt) {
		case Renderer::TextureSamplerFilter::Linear:
			filter = VK_FILTER_LINEAR;
			break;
		case Renderer::TextureSamplerFilter::Nearest:
			filter = VK_FILTER_NEAREST;
			break;
		}
		return filter;
	}
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter):_size(size)
	{		
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		std::vector<Vulkan::Texture> textures;
		
		TextureLoader::begin(context.device, context.commandBuffer, context.queue, context.memoryProperties)
			.addTexture(pfile)
			.setSamplerAddressMode(GetSamplerAddrMode(samplerAdd))
			.setSamplerFilter(GetSamplerFilter(filter))
			.load(textures);
		_texture = textures[0];
		switch (_texture.format) {
		case VK_FORMAT_R8_UNORM:
			_fmt = Renderer::TextureFormat::R8;
			break;
		case VK_FORMAT_R8G8_UNORM:
			_fmt = Renderer::TextureFormat::R8G8;
			break;
		case VK_FORMAT_R8G8B8_UNORM:
			_fmt = Renderer::TextureFormat::R8G8B8;
			break;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
			_fmt = Renderer::TextureFormat::R8G8B8A8;
			break;
		default:
			assert(0);
			break;
		}
		if (_size.x == -1 || _size.y == -1) {
			_size = glm::vec2(_texture.width, _texture.height);
		}
	}
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t* pixels, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter)
		:_size(glm::vec2(width,height))
	{
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		bool enableLod = false;
		int bytesperpixel = 0;
		_fmt = fmt;
		VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		switch (fmt) {
		case Renderer::TextureFormat::R8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			bytesperpixel = 1;
			format = VK_FORMAT_R8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			bytesperpixel = 2;
			format = VK_FORMAT_R8G8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8B8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			bytesperpixel = 3;
			format = VK_FORMAT_R8G8B8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			bytesperpixel = 4;
			format = VK_FORMAT_R8G8B8A8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::D32:
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			bytesperpixel = 4;
			format = VK_FORMAT_D32_SFLOAT;
			break;
		}
		
		
		ASSERT(format != VK_FORMAT_MAX_ENUM, "Unkown format for bytesperpixel: {0}", bytesperpixel);
		VkDeviceSize imageSize = (uint64_t)width * (uint64_t)height * bytesperpixel;
		TextureProperties props;
		props.format = format;
		props.aspect = aspect;
		props.height = height;
		props.width = width;
		props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		props.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		props.samplerProps.addressMode = GetSamplerAddrMode(samplerAdd);
		props.samplerProps.filter = GetSamplerFilter(filter);

		if (pixels != nullptr)
			props.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		props.mipLevels = enableLod ? 0 : 1;
#ifdef __USE__VMA__
		props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
		props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
		initTexture(context.device, context.memoryProperties, props, _texture);
		
		if (pixels != nullptr) {
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
		
	}
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter) 
		:_size(glm::vec2(width, height))
	{
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		bool enableLod = false;
		int bytesperpixel = 0;
		_fmt = fmt;
		VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		VkImageUsageFlagBits usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		VkFormat format = VK_FORMAT_MAX_ENUM;
		switch (fmt) {
		case Renderer::TextureFormat::R8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			bytesperpixel = 1;
			format = VK_FORMAT_R8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			bytesperpixel = 2;
			format = VK_FORMAT_R8G8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8B8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			bytesperpixel = 3;
			format = VK_FORMAT_R8G8B8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			bytesperpixel = 4;
			format = VK_FORMAT_R8G8B8A8_SRGB;// UNORM;
			break;
		case Renderer::TextureFormat::D32:
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			bytesperpixel = 4;
			format = VK_FORMAT_D32_SFLOAT;
			break;
		}
		ASSERT(format != VK_FORMAT_MAX_ENUM, "Unkown format for bytesperpixel: {0}", bytesperpixel);
		VkDeviceSize imageSize = (uint64_t)width * (uint64_t)height * bytesperpixel;
		TextureProperties props;
		props.format = format;
		props.aspect = aspect;
		props.height = height;
		props.width = width;
		props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		props.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage;
		props.mipLevels = enableLod ? 0 : 1;
		props.samplerProps.addressMode = GetSamplerAddrMode(samplerAdd);
		props.samplerProps.filter = GetSamplerFilter(filter);
#ifdef __USE__VMA__
		props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
		props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
		initTexture(context.device, context.memoryProperties, props, _texture);		
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
	void VulkanTextureImpl::SetName(const char* pname)
	{
		Vulkan::setTextureName(_texture, pname);
	}
	bool VulkanTextureImpl::SaveToFile(const char* ppath)
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		bool supportsBlit = (context.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && (context.formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

				Image dstImage;
			VkImageCreateInfo imageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			imageCI.format = _texture.format;
			imageCI.extent = { _texture.width, _texture.height,1 };
			imageCI.mipLevels = 1;
			imageCI.arrayLayers = 1;
			imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCI.tiling = VK_IMAGE_TILING_LINEAR;
			imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;// VK_IMAGE_LAYOUT_UNDEFINED;
			imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			initImage(context.device, imageCI, dstImage, true);
	
			int numpixels = 4;
			switch (_texture.format) {
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SRGB:
				numpixels = 1;
				break;
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SRGB:
				numpixels = 2;
				break;
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SRGB:
				numpixels = 3;
				break;
			}
	
	
	
			transitionImage(context.device, context.queue, context.commandBuffer, dstImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
			transitionImage(context.device, context.queue, context.commandBuffer, _texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	
			VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			VkResult res = vkBeginCommandBuffer(context.commandBuffer, &beginInfo);
			assert(res == VK_SUCCESS);
	
	
	
			if (supportsBlit) {
				VkOffset3D blitSize;
				blitSize.x = _texture.width;
				blitSize.y = _texture.height;
				blitSize.z = 1;
	
				VkImageBlit imageBlitRegion{};
				imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlitRegion.srcSubresource.layerCount = 1;
				imageBlitRegion.srcOffsets[1] = blitSize;
				imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlitRegion.dstSubresource.layerCount = 1;
				imageBlitRegion.dstOffsets[1];
	
				vkCmdBlitImage(context.commandBuffer, _texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&imageBlitRegion,
					VK_FILTER_NEAREST);
			}
			else {
				VkImageCopy imageCopyRegion{};
	
				imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopyRegion.srcSubresource.layerCount = 1;
				imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopyRegion.dstSubresource.layerCount = 1;
				imageCopyRegion.extent.width = _texture.width;
				imageCopyRegion.extent.height = _texture.height;
				imageCopyRegion.extent.depth = 1;
	
				vkCmdCopyImage(context.commandBuffer,
					_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&imageCopyRegion);
			}
	
			res = vkEndCommandBuffer(context.commandBuffer);
			assert(res == VK_SUCCESS);
	
			VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &context.commandBuffer;
	
			VkFence fence = initFence(context.device);
	
	
			res = vkQueueSubmit(context.queue, 1, &submitInfo, fence);
			assert(res == VK_SUCCESS);
	
			res = vkWaitForFences(context.device, 1, &fence, VK_TRUE, UINT64_MAX);
			assert(res == VK_SUCCESS);
	
	
			vkDestroyFence(context.device, fence, nullptr);
	
	
			transitionImage(context.device, context.queue, context.commandBuffer, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	
			//transitionImage(device, queue, cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcLayout);
			transitionImage(context.device, context.queue, context.commandBuffer, _texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			//generateMipMaps(context.device, context.queue, context.commandBuffer, _texture);

				VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT,0,0 };
			VkSubresourceLayout subResourceLayout;
			vkGetImageSubresourceLayout(context.device, dstImage.image, &subResource, &subResourceLayout);
	
			bool colorSwizzle = false;
			if (!supportsBlit)
			{
				std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
				colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), dstImage.format) != formatsBGR.end());
			}
	
			uint8_t* data{ nullptr };
	#ifdef __USE__VMA__
			data = (uint8_t*)dstImage.allocationInfo.pMappedData;
	#else
			vkMapMemory(device, dstImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	#endif
			data += subResourceLayout.offset;
	
			//std::string filename = std::to_string(index) + ".jpg";
			if (colorSwizzle) {
				uint32_t* ppixel = (uint32_t*)data;
				//must be a better way to do this
				for (uint32_t i = 0; i < _texture.height; i++) {
					for (uint32_t j = 0; j < _texture.width; j++) {
						if (numpixels == 4) {
							uint32_t pix = ppixel[i * _texture.width + j];
							uint8_t a = (pix & 0xFF000000) >> 24;
							uint8_t r = (pix & 0x00FF0000) >> 16;
							uint8_t g = (pix & 0x0000FF00) >> 8;
							uint8_t b = (pix & 0x000000FF);
							uint32_t newPix = (a << 24) | (b << 16) | (g << 8) | r;
							ppixel[i * _texture.width + j] = newPix;
						}
						else if (numpixels == 3) {
							uint8_t r = ppixel[i * _texture.width + j+0];
							uint8_t g = ppixel[i * _texture.width + j + 1];
							uint8_t b = ppixel[i * _texture.width + j + 2];
							ppixel[i * _texture.width + j+0] = b;
							ppixel[i * _texture.width + j + 0] = r;
						}
						else if (numpixels == 2) {
							assert(0);
						}
						else {

						}
	
					}
				}
			}
			stbi_write_jpg(ppath, _texture.width, _texture.height, numpixels, data, 100);
			cleanupImage(context.device, dstImage);
		return true;
	}
}
