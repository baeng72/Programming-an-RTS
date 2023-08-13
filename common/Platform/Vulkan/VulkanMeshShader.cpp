#pragma once
#include <glm/glm.hpp>
#include "VulkanMeshShader.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"

Renderer::MeshShader* Renderer::MeshShader::Create(Renderer::RenderDevice* pdevice,void*shaderData) {
	return new Vulkan::VulkanMeshShader(pdevice,shaderData);
}
namespace Vulkan {
	
	VulkanMeshShader::VulkanMeshShader(Renderer::RenderDevice*pdevice, void*shaderData){
		_pShaderData = (VulkanShaderData*)shaderData;
		_pdevice = pdevice;
	
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;

	
	}
	Vulkan::VulkanMeshShader::~VulkanMeshShader()
	{
		/*Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::cleanupPipeline(context.device, pipeline);
		Vulkan::cleanupPipelineLayout(context.device, pipelineLayout);*/

	}
	void VulkanMeshShader::Bind()
	{
		
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		
		vkCmdBindDescriptorSets(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipelineLayout, 0, 1, &_pShaderData->descriptorSet, 0, nullptr);
		vkCmdBindPipeline(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipeline);
	}

	void VulkanMeshShader::SetPushConstData(void* pdata, uint32_t size) {
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		
		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, pdata);
	}
	void VulkanMeshShader::SetUniformData(void*ptr, uint32_t len)
	{
		memcpy(_pShaderData->ubo, ptr, len);
	}
}
