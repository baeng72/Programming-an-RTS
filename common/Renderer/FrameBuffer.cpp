#include "FrameBuffer.h"

#include "../Core/Log.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLFrameBuffer.h"
#include "../Platform/Vulkan/VulkanFrameBuffer.h"

namespace Renderer {
	FrameBuffer* FrameBuffer::Create(RenderDevice* pdevice, Texture**pptextures,uint32_t count,Texture*pdepthmap,bool clearonrender,bool clonedevice) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLFrameBuffer(pdevice, pptextures,count,pdepthmap,clearonrender,clonedevice);
		case Core::API::Vulkan:
			return new Vulkan::VulkanFrameBuffer(pdevice, pptextures, count,pdepthmap,clearonrender,clonedevice);
		}
		assert(0);
		return nullptr;
	}
}