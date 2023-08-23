#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"

namespace Renderer {
	struct LineVertex {
		glm::vec3 position;
		glm::vec4 color;
	};
	class Line {
		
	public:
		static Line * Create(RenderDevice* pdevice,LineVertex*vertices,uint32_t vertexCount,float lineWidth=1);
		virtual ~Line() = default;		
		virtual void Draw(glm::mat4&viewProj) = 0;
		virtual void ResetVertices(Renderer::LineVertex* vertices, uint32_t vertexCount) = 0;
	};
}
