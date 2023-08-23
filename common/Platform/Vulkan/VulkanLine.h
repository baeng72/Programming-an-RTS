#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Line.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanLine : public Renderer::Line {
		Renderer::RenderDevice* _pdevice;
		struct PushConst {
			glm::mat4 projection;
			
		}pushConst;
		float _lineWidth;

		Vulkan::Buffer _vertexBuffer;
		uint32_t		_vertexCount;
		std::unique_ptr<VulkanVIBuffer> lineVertexPtr;		
		std::unique_ptr<VulkanPipelineLayout> linePipelineLayoutPtr;
		std::unique_ptr<VulkanPipeline> linePipelinePtr;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		
	public:
		VulkanLine(Renderer::RenderDevice* pdevice,Renderer::LineVertex*vertices,uint32_t vertexCount, float lineWidth);
		virtual ~VulkanLine();		
		virtual void Draw(glm::mat4&viewProj) override;
		virtual void ResetVertices(Renderer::LineVertex* vertices, uint32_t vertexCount)override;
	};
}
