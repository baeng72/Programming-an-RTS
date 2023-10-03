#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/ParticleSwarm.h"
#include "ShaderUtil.h"
namespace GL {
	class GLParticleSwarm : public Renderer::ParticleSwarm {
		ShaderUtil	_shader;
		GLuint _vao;
		GLuint _vertexBuffer;
		uint32_t _vertexCount;
		vec2 _size;
	public:
		GLParticleSwarm(Renderer::RenderDevice* pdevice, Renderer::ParticleVertex* vertices, uint32_t vertexCount, glm::vec2& size);
		virtual ~GLParticleSwarm();
		virtual void Draw(glm::mat4& viewProj, glm::vec3& eyePos) override;
		virtual void ResetVertices(Renderer::ParticleVertex* pvertices, uint32_t vertexCount)override;
	};
}