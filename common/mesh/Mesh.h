#pragma once

#include "../Renderer/RenderDevice.h"
#include "../Renderer/Shader.h"
namespace Mesh {
	class Mesh {		
	public:
		static Mesh* Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
		static Mesh * Create(Renderer::RenderDevice* pdevice,float*pvertices,uint32_t vertStride,uint32_t vertCount, uint32_t * pindices, uint32_t indSize,bool optimize=true);
		virtual ~Mesh() = default;
		virtual void Render(Renderer::Shader*pshader) = 0;
	};
}
