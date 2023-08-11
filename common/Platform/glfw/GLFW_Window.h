#pragma once

#include "../../Core/Window.h"
#include "../../Renderer/RenderDevice.h"
#include <GLFW/glfw3.h>
class GLFW_Window : public Window {
	GLFWwindow* _window;
	//
	struct WinData {
		std::string title;
		uint32_t _width;
		uint32_t _height;
		std::function<void(Event&)> _evFunc;
		
	}winData;
public:
	GLFW_Window(uint32_t width, uint32_t height,const std::string&title);
	virtual ~GLFW_Window();
	virtual void *GetNativeHandle()const override { return _window; }
	virtual void SetEventHandler(const std::function<void(Event&)>& func)override;
	virtual void OnBeginUpdate()override;
	virtual void OnUpdate()override;
	virtual bool IsKeyPressed(int key)override { return glfwGetKey(_window, key)==GLFW_PRESS; }
};
