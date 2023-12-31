#pragma once

#include "../Renderer/RenderDevice.h"

#include "../Renderer/ShaderTypes.h"
#include "MeshTypes.h"
#include "BaseMesh.h"
namespace Mesh {
	class Mesh : public BaseMesh {		
	public:
		static Mesh* Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize,Renderer::VertexAttributes&attributes,bool optimize=false);
		//static Mesh * Create(Renderer::RenderDevice* pdevice,float*pvertices,uint32_t vertStride,uint32_t vertSize, uint32_t * pindices, uint32_t indSize,bool optimize=true);
		//virtual ~Mesh() = default;
		virtual void Render() = 0;
		virtual void* GetNativeHandle() = 0;
	};
}
