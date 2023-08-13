#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "ShaderManager.h"
namespace Renderer {
	
	class MeshShader {
	public:
		
		static MeshShader * Create(RenderDevice* pdevice,void*shaderData);
		virtual ~MeshShader() = default;
		virtual void Bind() = 0;
		virtual void SetPushConstData(void*, uint32_t len)=0;
		virtual void SetUniformData(void*, uint32_t len) = 0;
	};
}
