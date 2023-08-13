#pragma once

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
		VkCommandBuffer		_cmd;
		bool				_enableVSync;
		bool				_enableGeometry;
		bool				_enableLines;
	public:
		VulkanRenderDevice(void* nativeWindowHandle);
		virtual ~VulkanRenderDevice();
		virtual void Init() override;
		virtual void StartRender() override;
		virtual void EndRender() override;
		virtual void SetVSync(bool vsync) override;
		virtual void SetGeometry(bool geom) override;
		virtual void SetLines(bool lines)override;
		virtual void* GetDeviceContext()const override;
		virtual void* GetCurrentFrameData()const override;
		virtual void Wait()const override;
		virtual void  GetDimensions(int* width, int* height)const override;
	};
}
