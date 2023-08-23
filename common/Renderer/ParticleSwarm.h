#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"

namespace Renderer {
	struct ParticleVertex {
		glm::vec3 position;
		glm::vec4 color;
	};
	class ParticleSwarm {
		
	public:
		static ParticleSwarm* Create(RenderDevice* pdevice,ParticleVertex*vertices,uint32_t vertexCount,glm::vec2&size);
		virtual ~ParticleSwarm() = default;		
		virtual void Draw(glm::mat4&viewProj,glm::vec3&eyePos) = 0;
		virtual void ResetVertices(ParticleVertex* pvertices, uint32_t vertexCount)=0;
	};
}
