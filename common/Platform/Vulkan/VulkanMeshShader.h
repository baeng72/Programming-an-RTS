#pragma once
#include <glm/glm.hpp>
#include "../../Renderer/MeshShader.h"

#include "VulkanEx.h"
#include "VulkanShaderManager.h"
namespace Vulkan {
	
	class VulkanMeshShader : public Renderer::MeshShader {
		Renderer::RenderDevice* _pdevice;
		VulkanShaderData* _pShaderData;
	public:
		VulkanMeshShader(Renderer::RenderDevice* pdevice,void*shaderData);
		virtual ~VulkanMeshShader();
		void Bind()override;
		virtual void SetPushConstData(void*, uint32_t len)override;
		virtual void SetUniformData(void*, uint32_t len)override;//won't scale to multiple sets or bindings 
	};
}
