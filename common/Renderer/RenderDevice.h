#pragma once
#include <memory>
#include "../Core/defines.h"
#include "../Core/types.h"

namespace Renderer {

	class RenderDevice {
	public:
		virtual ~RenderDevice() = default;
		virtual void Init() = 0;
		virtual void StartRender(bool mainpass=true) = 0;
		virtual void EndRender() = 0;
		virtual void StartOffscreenRender() = 0;
		virtual void EndOffscreenRender()=0;
		/*virtual void StartShadowRender() = 0;
		virtual void EndShadowRender() = 0;*/
		virtual void SetVSync(bool vsync) = 0;
		virtual void EnableGeometry(bool geom) = 0;
		virtual void EnableWireframe(bool wireframe) = 0;
		virtual void EnableLines(bool lines) = 0;
		virtual void EnableDepthBuffer(bool depth) = 0;
		virtual void* GetDeviceContext()const = 0;
		virtual void* GetCurrentFrameData()const = 0;
		virtual void* GetDefaultResources()const = 0;
		virtual void  GetDimensions(int* width, int* height)const = 0;
		virtual void  Wait()const = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear(Rect& r, Color clr) = 0;
		virtual void SetViewport(ViewPort& vp)=0;
		virtual float GetCurrentTicks() = 0;
		virtual void DrawVertices(uint32_t count,uint32_t offset=0) = 0;
		static RenderDevice* Create(void* nativeWidowHandle);
	};
}