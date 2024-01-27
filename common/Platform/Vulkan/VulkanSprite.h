#pragma once
#include "../../Core/defines.h"
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Buffer.h"
#include "../../Renderer/Sprite.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanSprite : public Renderer::Sprite {
		Renderer::RenderDevice* _pdevice;
		std::unique_ptr<Renderer::Buffer> _uboBuffer;
		struct UBO {
			mat4 ortho;
		}*_pubo;
		struct PushConst {			
			//glm::mat4 ortho;
			glm::mat4 model;//scale
			Color color;
			vec2 uvs[4];
			
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
		virtual void Draw(Renderer::Texture* ptexture, Rect& r, vec3& position,Color&color)override;
		/*virtual void SetScale(vec2 scale) override {
			_scale = scale;
			_scaleoverriden = true;
		}*/
	};
}
