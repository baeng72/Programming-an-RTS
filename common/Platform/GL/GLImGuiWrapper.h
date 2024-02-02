#pragma once 
#include "../../Renderer/RenderDevice.h"
#include "../../Core/Window.h"
#include "../../Core/Event.h"
#include "../imgui/ImGuiWrapper.h"
#include <glad/glad.h>
#include "ShaderUtil.h"
namespace GL {
	class GLImGuiWrapper :public ImGui::ImGuiWrapper {
		Renderer::RenderDevice* _pdevice;
		Core::Window* _pwindow;
		GLuint _vao;
		struct FrameData {
			GLuint vao;
			GLuint vertexBuffer;
			GLuint indexBuffer;
			uint32_t vertSize;
			uint32_t indSize;
			uint32_t numIndices;
		};
		FrameData frames[MAX_FRAMES];//double-buffering
		uint32_t currFrame;
		GLuint _texture;		
		ShaderUtil	_shader;
		ViewPort _viewport;
		void OnEvent(Event& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		bool OnKeyReleased(KeyReleasedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnMouseButtonDown(MouseButtonPressedEvent& e);
		bool OnMouseButtonUp(MouseButtonReleasedEvent& e);
		bool OnKeyTyped(KeyTypedEvent& e);
		GLint _texloc;
		GLint _matloc;
	public:
		GLImGuiWrapper(Renderer::RenderDevice* pdevice, Core::Window* pwindow);
		virtual ~GLImGuiWrapper();
		
		virtual void Update(float delta)override;
		virtual void SetViewport(ViewPort& vp)override;
		virtual void Start()override;
		virtual void End()override;
	};
}