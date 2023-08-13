#pragma once
#include <glm/glm.hpp>
#include "../../Renderer/ShaderManager.h"
#include "VulkanEx.h"
namespace Vulkan {
	struct VulkanShaderData {
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet  descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline		 pipeline;
		void* ubo;		
	};
	class VulkanShaderManager : public  Renderer::ShaderManager {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _uniformBuffer;
		std::vector<Vulkan::UniformBufferInfo> _uboInfo;
		std::vector<VulkanShaderData> _shaderList;
		std::vector<VulkanShaderData> _wireframeShaderList;
		void CompileShaders();
	public:		
		VulkanShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~VulkanShaderManager();
		virtual void* GetShaderData(ShaderType type,bool wireframe) override;
	};
}
