#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "MeshShader.h"
namespace Renderer {
	class Mesh {
		RenderDevice* _pdevice;
	public:
		static Mesh * Create(RenderDevice* pdevice,float*pvertices,uint32_t vertSize,uint32_t * pindices, uint32_t indSize);
		virtual ~Mesh() = default;
		virtual void Render(MeshShader*pshader) = 0;
	};
}
