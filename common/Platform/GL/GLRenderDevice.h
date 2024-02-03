#pragma once
#include "../../Core/Log.h"
#include "../../Core/profiler.h"
#include "../../Renderer/RenderDevice.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
namespace GL {

	class GLRenderDevice : public Renderer::RenderDevice {
		GLFWwindow* _window;
		float _clearColors[4];
		bool _depthTest;
		GLuint _vao;
	public:
		GLRenderDevice(void* nativeWindowHandle);
		virtual ~GLRenderDevice();
		virtual void Init() override;
		virtual void StartRender(bool mainpass=true) override;
		virtual void EndRender() override;
		virtual void StartOffscreenRender() override;
		virtual void EndOffscreenRender() override;
		/*virtual void StartShadowRender() override;
		virtual void EndShadowRender() override;*/
		virtual void SetVSync(bool vsync) override;
		virtual void EnableGeometry(bool geom) override;
		virtual void EnableWireframe(bool wireframe) override;
		virtual void EnableLines(bool lines)override;
		virtual void EnableDepthBuffer(bool depth)override;
		virtual void* GetDeviceContext()const override;
		virtual void* GetCurrentFrameData()const override;
		virtual void* GetDefaultResources()const override;
		virtual void Wait()const override;
		virtual void  GetDimensions(int* width, int* height)const override;
		virtual void SetClearColor(float r, float g, float b, float a) override;
		virtual void Clear(Rect& r, Color clr) override;
		virtual void SetViewport(ViewPort& vp)override;
		virtual float GetCurrentTicks()override;
		virtual void DrawVertices(uint32_t count, uint32_t offset = 0)override;
		//virtual void DrawIndexed(uint32_t indexCount)override;

	};
}