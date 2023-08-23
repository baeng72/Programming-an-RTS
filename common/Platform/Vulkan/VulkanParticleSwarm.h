#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/ParticleSwarm.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanParticleSwarm : public Renderer::ParticleSwarm {
		Renderer::RenderDevice* _pdevice;
		struct PushConst {
			glm::mat4 viewProj;
			glm::vec3 eyePos;
			float padding0;
			glm::vec2 size;	//alignment issues?	
		}pushConst;
		Vulkan::Buffer _vertexBuffer;
		uint32_t		_vertexCount;
		std::unique_ptr<VulkanVIBuffer> particleVertexPtr;		
		std::unique_ptr<VulkanPipelineLayout> particlePipelineLayoutPtr;
		std::unique_ptr<VulkanPipeline> particlePipelinePtr;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		
	public:
		VulkanParticleSwarm(Renderer::RenderDevice* pdevice,Renderer::ParticleVertex*vertices,uint32_t vertexCount,glm::vec2&size);
		virtual ~VulkanParticleSwarm();		
		virtual void Draw(glm::mat4&viewProj,glm::vec3&eyePos) override;
		virtual void ResetVertices(Renderer::ParticleVertex* pvertices, uint32_t vertexCount)override;
	};
}
