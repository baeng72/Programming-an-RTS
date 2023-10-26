#pragma once
#include "RenderDevice.h"
#include "ShaderTypes.h"

namespace Renderer {
	class VIBuffer {
	public:
		static VIBuffer*Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes, bool optimize = false);
		virtual ~VIBuffer() = default;
		virtual void Bind() = 0;
		virtual void Render()=0;
		virtual size_t GetHash() = 0;
	};
}