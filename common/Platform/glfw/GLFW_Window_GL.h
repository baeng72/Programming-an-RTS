#pragma once
#include "../../Core/Window.h"
#include "../../Renderer/RenderDevice.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
namespace GLFW {
	class GLFW_Window_GL : public Core::Window {
		GLFWwindow* _window;
		//
		struct WinData {
			std::string title;
			uint32_t _width;
			uint32_t _height;
			std::vector<std::function<void(Event&)>> _evFunc;
			float xoffset;
			float yoffset;

		}winData;
		void SetWindowCenter();
	public:
		GLFW_Window_GL(uint32_t width, uint32_t height, const std::string& title);
		virtual ~GLFW_Window_GL();
		virtual void* GetNativeHandle()const override { return _window; }
		virtual void SetEventHandler(const std::function<void(Event&)>& func)override;
		virtual void OnBeginUpdate()override;
		virtual void OnUpdate()override;
		virtual bool IsKeyPressed(int key)override {
			//bool pressed = winData.keys.find(key) != winData.keys.end();
			//winData.keys.erase(key);
			//return pressed;
			//return _winData.keys.find(key)!=_winData.keys
			return glfwGetKey(_window, key) == GLFW_PRESS;
		}
		virtual void GetWindowSize(int& width, int& height)override {
			glfwGetWindowSize(_window, &width, &height);
		}
		virtual void GetCursorPos(float& xpos, float& ypos) override {
			double dxpos, dypos;
			glfwGetCursorPos(_window, &dxpos, &dypos);
			xpos = (float)dxpos;
			ypos = (float)dypos;
		}
		virtual bool IsMouseButtonPressed(int button) override {
			return glfwGetMouseButton(_window, button) == GLFW_PRESS;
		}
		virtual void GetScrollPos(float& xoffset, float& yoffset) override {
			xoffset = winData.xoffset;
			yoffset = winData.yoffset;
			winData.xoffset = winData.yoffset = 0.f;
		}
		virtual void ShowCursor(bool show) override {
			glfwSetInputMode(_window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
		}
		virtual void SetTitle(const char* ptitle)override {
			glfwSetWindowTitle(_window, ptitle);
		}
		virtual void GetWindowPos(int& xpos, int& ypos)override {
			glfwGetWindowPos(_window, &xpos, &ypos);
		}
	};
}