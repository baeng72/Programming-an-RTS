#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Sprite.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanSprite : public Renderer::Sprite {
		Renderer::RenderDevice* _pdevice;
		struct PushConst {
			glm::mat4 proj;
			glm::mat4 model;
		};
		int _width, _height;
		glm::vec2 _scale;
		glm::mat4 _orthoproj;
		Vulkan::Texture* _ptexture;				
		VkDescriptorSetLayout	descriptorLayout;
		VkDescriptorSet			descriptorSet;
		VkPipelineLayout		pipelineLayout;
		VkPipeline				pipeline;
		std::unique_ptr<VulkanDescriptor> spriteDescriptorPtr;
		static std::unique_ptr<VulkanPipelineLayout> spritePipelineLayoutPtr;
		static std::unique_ptr<VulkanPipeline> spritePipelinePtr;
		static int instances;
	public:
		VulkanSprite(Renderer::RenderDevice* pdevice);
		virtual ~VulkanSprite();		
		virtual void Draw(Renderer::Texture* ptexture, glm::vec3 position) override;
	};
}
