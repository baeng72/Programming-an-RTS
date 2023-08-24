#include "GLFW_Window.h"
#include "../../Core/Event.h"
//#include "../../Platform/Vulkan/VulkanRenderContext.h"
#include <iostream>
#include <cassert>
static bool glfwInitialized = false;
static void GLFWErrorCallback(int err, const char* description) {
	std::cerr << "GLFW Error " << err << ": " << description << std::endl;
}

Window* Window::Create(uint32_t width, uint32_t height, const std::string& title) {
	auto p = new GLFW_Window(width, height, title);
	//return std::make_shared<GLFW_Window>(width, height, title);
	return p;
}

GLFW_Window::GLFW_Window(uint32_t width, uint32_t height,const  std::string&title)
	:winData{title,width,height}
{
	winData.xoffset = winData.yoffset = 0.f;

	if (!glfwInitialized) {
		int success = glfwInit();
		assert(success);
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwInitialized = true;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	//_context = std::unique_ptr<RenderContext>(new VulkanRenderContext(_window));
	//_context->Init();
	glfwSetWindowUserPointer(_window, &winData);
	glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);

		data._width = width;
		data._height = height;
		WindowResizeEvent event(width, height);
		data._evFunc(event);
		
		/*WindowResizeEvent event(width, height);
		data.EventCallback(event);*/

		});
	glfwSetWindowCloseCallback(_window, [](GLFWwindow* window) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent event;
		data._evFunc(event);
		});
	glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int modes) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
		switch (action) {
		case GLFW_PRESS: {
			KeyPressedEvent event(key, false);
			data._evFunc(event);
			//data.keys.insert(key);
		}
					   break;
		case GLFW_RELEASE: {
			KeyReleasedEvent event(key);
			data._evFunc(event);
		}
						 break;
		case GLFW_REPEAT:
		{
			KeyPressedEvent event(key, true);
			data._evFunc(event);
		}
		break;
		}
		});
	glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int modes) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);

		switch (action) {
		case GLFW_PRESS:
		{
			MouseButtonPressedEvent event(button);
			data._evFunc(event);
		}
		break;
		case GLFW_RELEASE:
		{
			MouseButtonReleasedEvent event(button);
			data._evFunc(event);
		}
		break;
		}
		});

	glfwSetScrollCallback(_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
		data.xoffset = (float)xoffset;
		data.yoffset = (float)yoffset;
		MouseScrolledEvent event((float)xoffset, (float)yoffset);
		data._evFunc(event);
		});

	glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xPos, double yPos) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
		MouseMovedEvent event((float)xPos, (float)yPos);
		data._evFunc(event);

		});

	glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int keycode) {
		WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
		KeyTypedEvent event(keycode);
		data._evFunc(event);

		});
}

GLFW_Window::~GLFW_Window()
{
	glfwDestroyWindow(_window);
}

void GLFW_Window::SetEventHandler(const std::function<void(Event&)>& func)
{
	winData._evFunc = func;	
}

void GLFW_Window::OnBeginUpdate() {
	//_context->StartRender();
}

void GLFW_Window::OnUpdate()
{
	glfwPollEvents();
	//_context->EndRender();
}
