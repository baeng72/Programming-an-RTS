#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Texture.h"
namespace Renderer {
	class Sprite {		
	public:
		static Sprite* Create(RenderDevice* pdevice);
		virtual ~Sprite() = default;
		
		virtual void Draw(Texture* ptexture, vec3 position) = 0;

	};
}
