#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Texture.h"
namespace Renderer {
	class Sprite {
		RenderDevice* _pdevice;
	public:
		static Sprite* Create(RenderDevice* pdevice);
		virtual ~Sprite() = default;
		
		virtual void Draw(Texture* ptexture, glm::vec3 position) = 0;

	};
}
