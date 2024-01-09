#pragma once
#include "../../Core/defines.h"
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
		//glm::vec2 _scale;
		//bool _scaleoverriden;
		mat4 _xform;
		mat4 _orthoproj;		
		uint32_t _descriptorIndex;
		Vulkan::Texture* _ptexture;				
		VkDescriptorSetLayout	_descriptorLayout;
		//VkDescriptorSet			descriptorSet;
		VkDescriptorSet			_descriptorSets[MAX_FRAMES];
		VkPipelineLayout		_pipelineLayout;
		VkPipeline				_pipeline;
		std::unique_ptr<VulkanDescriptorList> spriteDescriptorPtr;
		static std::unique_ptr<VulkanPipelineLayout> spritePipelineLayoutPtr;
		static std::unique_ptr<VulkanPipeline> spritePipelinePtr;
		static int instances;
	public:
		VulkanSprite(Renderer::RenderDevice* pdevice);
		virtual ~VulkanSprite();		
		virtual void SetTransform(mat4& xform)override {_xform = xform;}
		virtual void Draw(Renderer::Texture* ptexture, vec3& position) override;
		/*virtual void SetScale(vec2 scale) override {
			_scale = scale;
			_scaleoverriden = true;
		}*/
	};
}
