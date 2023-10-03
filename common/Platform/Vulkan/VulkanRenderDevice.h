#pragma once
#include "../../Core/profiler.h"
#include "../../Renderer/RenderDevice.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
namespace Vulkan {
	struct VulkContext;

	class VulkanRenderDevice : public Renderer::RenderDevice {
		GLFWwindow* _window;
		std::unique_ptr<VulkState> _state;
		std::unique_ptr<VulkSwapchain> _swapchain;
		VulkContext			_context;
		VulkFrameData		_frameData;
		VulkFrameData		_shadowFrameData;
		struct ShadowData {
			VkRenderPass renderPass;
			VkFramebuffer frameBuffer;
			Vulkan::Texture shadowMap;
		}_shadowData;
		VkCommandBuffer		_cmd;
		bool				_enableVSync;
		bool				_enableGeometry;
		bool				_enableLines;
		bool				_enableWireframe;
		bool				_enableDepthBuffer;
	public:
		VulkanRenderDevice(void* nativeWindowHandle);
		virtual ~VulkanRenderDevice();
		virtual void Init() override;
		virtual void StartRender() override;
		virtual void EndRender() override;
		virtual void StartShadowRender() override;
		virtual void EndShadowRender() override;
		virtual void SetVSync(bool vsync) override;
		virtual void EnableGeometry(bool geom) override;
		virtual void EnableWireframe(bool wireframe) override;
		virtual void EnableLines(bool lines)override;
		virtual void EnableDepthBuffer(bool depth)override;
		virtual void* GetDeviceContext()const override;
		virtual void* GetCurrentFrameData()const override;
		virtual void Wait()const override;
		virtual void  GetDimensions(int* width, int* height)const override;
		virtual void SetClearColor(float r, float g, float b, float a) override;
		virtual void Clear(Rect& r, Color clr) override;
		virtual void SetViewport(ViewPort& vp)override;
		virtual float GetCurrentTicks()override { return (float)glfwGetTime(); }
		
	};
}
