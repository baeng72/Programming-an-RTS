#pragma once
#include <memory>
namespace Renderer {
	class RenderDevice {
	public:
		virtual ~RenderDevice() = default;
		virtual void Init() = 0;
		virtual void StartRender() = 0;
		virtual void EndRender() = 0;
		virtual void SetVSync(bool vsync) = 0;
		virtual void SetGeometry(bool geom) = 0;
		virtual void SetLines(bool lines) = 0;
		virtual void* GetDeviceContext()const = 0;
		virtual void* GetCurrentFrameData()const = 0;
		virtual void  GetDimensions(int* width, int* height)const = 0;
		virtual void  Wait()const = 0;
		static RenderDevice* Create(void* nativeWidowHandle);
	};
}