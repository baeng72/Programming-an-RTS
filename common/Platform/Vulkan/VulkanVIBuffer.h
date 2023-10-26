#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/VIBuffer.h"
#include "../../Renderer/ShaderTypes.h"
#include "../../common/Platform/Vulkan/VulkanEx.h"
#include "../../common/Platform/Vulkan/VulkState.h"
#include "../../common/Platform/Vulkan/VulkSwapchain.h"
namespace Vulkan {
	class VulkanVIBufferImpl : public Renderer::VIBuffer {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		uint32_t		_indexCount;
		size_t	_hash;
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes);
	public:
		VulkanVIBufferImpl(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes, bool optimize = false);
		virtual ~VulkanVIBufferImpl();
		virtual void Bind()override;
		virtual size_t GetHash()override;
		virtual void Render()override;
	};
}