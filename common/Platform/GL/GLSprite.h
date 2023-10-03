#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Sprite.h"
#include "ShaderUtil.h"
namespace GL {
	class GLSprite : public Renderer::Sprite {
		Renderer::RenderDevice* _pdevice;
		ShaderUtil _shader;
		GLuint _vao;
		glm::vec2 _scale;
		glm::mat4 _orthoproj;
	public:
		GLSprite(Renderer::RenderDevice* pdevice);
		virtual ~GLSprite();
		virtual void Draw(Renderer::Texture * ptexture, vec3 position)override;
	};
}