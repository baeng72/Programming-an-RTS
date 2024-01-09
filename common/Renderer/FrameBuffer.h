#pragma once

#include "RenderDevice.h"
#include "Texture.h"
namespace Renderer {
	union ClearColor {
		vec4 color;
		struct {
			float depth;
			uint32_t stencil;
		}depthstencil;
	};
	class FrameBuffer {
	protected:
		Renderer::RenderDevice* _pdevice;
		Renderer::Texture* _ptexture;
	public:
		static FrameBuffer* Create(RenderDevice*pdevice,Texture**pptextures,uint32_t count,Texture*pdepthmap=nullptr,bool clearonrender=true,bool clonedevice=false);
		virtual ~FrameBuffer() = default;
		virtual void StartRender() =0;
		virtual void EndRender() =0;
		virtual void* GetContext() = 0;
		virtual void* GetNativeHandle() = 0;
		virtual Texture* GetTexture() = 0;
		virtual void SetClearColor(ClearColor* pclrs, uint32_t count) = 0;
		virtual void DrawVertices(uint32_t count) = 0;
		virtual void Clear(Rect& r, Color clr) = 0;
	};
}