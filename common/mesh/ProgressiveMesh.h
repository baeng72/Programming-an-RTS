#pragma once
#include "../Renderer/RenderDevice.h"
#include "Mesh.h"

namespace Mesh {

	class ProgressiveMesh {
	public:
		static ProgressiveMesh* Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize,uint32_t vertStride, uint32_t* pindices, uint32_t indSize);
		virtual ~ProgressiveMesh() = default;
		virtual void SetIndexCount(uint32_t indexCount) = 0;
		virtual uint32_t GetIndexCount() = 0;
		virtual void Render(Renderer::Shader* pshader) = 0;
	};
}