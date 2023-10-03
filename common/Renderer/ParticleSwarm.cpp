#include "ParticleSwarm.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanParticleSwarm.h"
#include "../Platform/GL/GLParticleSwarm.h"

Renderer::ParticleSwarm* Renderer::ParticleSwarm::Create(Renderer::RenderDevice* pdevice, Renderer::ParticleVertex* pvertices, uint32_t vertexCount, glm::vec2& size) {
	switch (Core::GetAPI()) {
	case Core::API::GL:
		return new GL::GLParticleSwarm(pdevice, pvertices, vertexCount, size);
	case Core::API::Vulkan:
		return new Vulkan::VulkanParticleSwarm(pdevice, pvertices, vertexCount, size);
	}
	assert(0);
	return nullptr;
	
}

