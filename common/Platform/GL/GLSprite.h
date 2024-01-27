#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Sprite.h"
#include "ShaderUtil.h"
namespace GL {
	class GLSprite : public Renderer::Sprite {
		Renderer::RenderDevice* _pdevice;
		ShaderUtil _shader;
		GLuint _vao;
		mat4 _xform;
		mat4 _orthoproj;
	public:
		GLSprite(Renderer::RenderDevice* pdevice);
		virtual ~GLSprite();
		virtual void SetTransform(mat4& xform)override { _xform = xform; }
		virtual void Draw(Renderer::Texture * ptexture, vec3& position)override;
		virtual void Draw(Renderer::Texture* ptexture, Rect& r, vec3& position,Color&color)override;
		/*virtual void SetScale(vec2 scale) override {
			_scale = scale;
		}*/
	};
}