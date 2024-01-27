#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Texture.h"
namespace Renderer {
	class Sprite {		
	public:
		static Sprite* Create(RenderDevice* pdevice);
		virtual ~Sprite() = default;
		//virtual void SetScale(vec2 scale) = 0;
		virtual void SetTransform(mat4& xform) = 0;
		virtual void Draw(Texture* ptexture, vec3& position) = 0;
		virtual void Draw(Texture* ptexture, Rect& r, vec3& position,Color&color) = 0;
	};
}
