#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"

namespace Renderer {	

	class Line2D {//line in screen space
	public:
		static Line2D* Create(RenderDevice* pdevice);
		virtual ~Line2D() = default;
		virtual void Draw(vec2* pvertices, uint32_t count, vec4 color, float width = 1) = 0;
		virtual void Update(int width, int height)=0;
	};
}
