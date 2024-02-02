#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Core/Window.h"
#include "../../Core/Event.h"
#include "../imgui/ImGuiWrapper.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanImGuiWrapper :public ImGui::ImGuiWrapper {
		Renderer::RenderDevice* _pdevice;
		Core::Window* _pwindow;
		struct PushConst {
			glm::vec2 scale;
			glm::vec2 translate;
		}; 
		
		struct FrameData {
			Buffer vertexBuffer;
			Buffer indexBuffer;
			VkDeviceSize vertSize;
			VkDeviceSize indSize;
			uint8_t* pverts;
			uint8_t* pindices;
			uint32_t numIndices;
		};
		FrameData frames[MAX_FRAMES];//double-buffering
		uint32_t currFrame;
		std::unique_ptr<VulkanTexture> fontTexturePtr;
		std::unique_ptr<VulkanDescriptor> fontDescriptorPtr;
		std::unique_ptr<VulkanPipelineLayout> fontPipelineLayoutPtr;
		std::unique_ptr<VulkanPipeline> fontPipelinePtr;
		ViewPort _viewport;
		void OnEvent(Event& e);
		bool OnKeyPressed(KeyPressedEvent& e);
		
		bool OnKeyReleased(KeyReleasedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnMouseButtonDown(MouseButtonPressedEvent& e);
		bool OnMouseButtonUp(MouseButtonReleasedEvent& e);
		bool OnKeyTyped(KeyTypedEvent& e);
	public:
		VulkanImGuiWrapper(Renderer::RenderDevice* pdevice, Core::Window* pwindow);
		virtual ~VulkanImGuiWrapper();
		
		virtual void Update(float delta)override;
		virtual void SetViewport(ViewPort& vp)override;
		virtual void Start()override;
		virtual void End()override;
	};
}