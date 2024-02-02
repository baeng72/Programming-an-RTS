#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Core/Window.h"
#include <imgui.h>
namespace ImGui {
	ImGuiKey KeycodeToImGuiKey(int32_t keyCode);
	class ImGuiWrapper {
	public:
		static ImGuiWrapper* Create(Renderer::RenderDevice* pdevice, Core::Window* pwindow);
		virtual ~ImGuiWrapper() = default;
		
		virtual void Update(float delta)=0;
		virtual void SetViewport(ViewPort& vp)=0;
		virtual void Start() = 0;
		virtual void End()=0;
	};
}