#include "FrameBuffer.h"

#include "../Core/Log.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLFrameBuffer.h"
#include "../Platform/Vulkan/VulkanFrameBuffer.h"

namespace Renderer {
	FrameBuffer* FrameBuffer::Create(RenderDevice* pdevice, Texture**pptextures,uint32_t count,bool clearonrender) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLFrameBuffer(pdevice, pptextures,count,clearonrender);
		case Core::API::Vulkan:
			return new Vulkan::VulkanFrameBuffer(pdevice, pptextures, count,clearonrender);
		}
		assert(0);
		return nullptr;
	}
}