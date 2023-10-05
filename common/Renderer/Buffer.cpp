#include "Buffer.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLBuffer.h"
#include "../Platform/Vulkan/VulkanBuffer.h"
namespace Renderer {
	Buffer* Buffer::Create(RenderDevice* pdevice, uint32_t size, uint32_t count, bool isUniform, bool isDynamic) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLBuffer(pdevice, nullptr, size, count, isUniform, isDynamic);
		case Core::API::Vulkan:
			return new Vulkan::VulkanBufferImpl(pdevice, nullptr, size, count, isUniform, isDynamic);
		}
		assert(0);
		return nullptr;
	}
	Buffer* Buffer::Create(RenderDevice* pdevice, void* ptr, uint32_t size, uint32_t count, bool isUniform, bool isDynamic) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLBuffer(pdevice, ptr, size, count, isUniform, isDynamic);
		case Core::API::Vulkan:
			return new Vulkan::VulkanBufferImpl(pdevice, ptr, size, count, isUniform, isDynamic);
		}
		assert(0);
		return nullptr;
	}
}