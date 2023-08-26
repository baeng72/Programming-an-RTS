#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Line2D.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanLine2D : public Renderer::Line2D {
		Renderer::RenderDevice* _pdevice;
		struct PushConst {
			glm::mat4 projection;
			glm::vec2 start;
			glm::vec2 end;
			glm::vec4 color;			
		}pushConst;
		
		std::unique_ptr<VulkanPipelineLayout> linePipelineLayoutPtr;
		std::unique_ptr<VulkanPipeline> linePipelinePtr;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		
	public:
		VulkanLine2D(Renderer::RenderDevice* pdevice);
		virtual ~VulkanLine2D();		
		virtual void Draw(vec2* pvertices, uint32_t count, vec4 color, float width = 1) override;		
		virtual void Update(int width, int height)override;
	};
}
