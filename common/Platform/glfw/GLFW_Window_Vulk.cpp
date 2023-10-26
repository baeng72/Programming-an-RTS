#include "GLFW_Window_Vulk.h"
#include "../../Core/Event.h"
#include "../../Core/Log.h"
//#include "../../Platform/Vulkan/VulkanRenderContext.h"
#include <iostream>
#include <cassert>
namespace GLFW {

	GLFW_Window_Vulk::GLFW_Window_Vulk(uint32_t width, uint32_t height, const  std::string& title)
		:winData{ title,width,height }
	{
		LOG_INFO("Vulkan");
		static bool glfwInitialized = false;
		winData.xoffset = winData.yoffset = 0.f;
		LOG_INFO("Initializing Vulkan GLFW Window...");
		if (!glfwInitialized) {
			int success = glfwInit();
			assert(success);
			glfwSetErrorCallback([](int err, const char* description) {
				LOG_INFO("GLFW Error {0}-{1}", err, description);
				});
			glfwInitialized = true;
		}
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			LOG_CRITICAL("GLFW doesn't support Vulkan. Exiting!");
			assert(0);
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		assert(_window);

		glfwSetWindowUserPointer(_window, &winData);
		glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);

			data._width = width;
			data._height = height;
			WindowResizeEvent event(width, height);
			for (auto& func : data._evFunc)
				func(event);

			/*WindowResizeEvent event(width, height);
			data.EventCallback(event);*/

			});
		glfwSetWindowCloseCallback(_window, [](GLFWwindow* window) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			for (auto& func : data._evFunc)
				func(event);
			});
		glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int modes) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS: {
				KeyPressedEvent event(key,scancode,modes, false);
				for (auto& func : data._evFunc)
					func(event);
				//data.keys.insert(key);
			}
						   break;
			case GLFW_RELEASE: {
				KeyReleasedEvent event(key,scancode,modes);
				for (auto& func : data._evFunc)
					func(event);
			}
							 break;
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key,scancode,modes, true);
				for (auto& func : data._evFunc)
					func(event);
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
				for (auto& func : data._evFunc)
					func(event);
			}
			break;
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				for (auto& func : data._evFunc)
					func(event);
			}
			break;
			}
			});

		glfwSetScrollCallback(_window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
			data.xoffset = (float)xoffset;
			data.yoffset = (float)yoffset;
			MouseScrolledEvent event((float)xoffset, (float)yoffset);
			for (auto& func : data._evFunc)
				func(event);
			});

		glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xPos, double yPos) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xPos, (float)yPos);
			for (auto& func : data._evFunc)
				func(event);

			});

		glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int keycode) {
			WinData& data = *(WinData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(keycode);
			for (auto& func : data._evFunc)
				func(event);

			});
		SetWindowCenter();
	}

	GLFW_Window_Vulk::~GLFW_Window_Vulk()
	{
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void GLFW_Window_Vulk::SetEventHandler(const std::function<void(Event&)>& func)
	{
		winData._evFunc.push_back(func);
	}

	void GLFW_Window_Vulk::OnBeginUpdate() {
		//_context->StartRender();
	}

	void GLFW_Window_Vulk::OnUpdate()
	{
		glfwPollEvents();
		//_context->EndRender();
	}

	void GLFW_Window_Vulk::SetWindowCenter()
	{
		// Get window position and size
		int posX, posY;
		glfwGetWindowPos(_window, &posX, &posY);

		int width, height;
		glfwGetWindowSize(_window, &width, &height);

		// Halve the window size and use it to adjust the window position to the center of the window
		width >>= 1;
		height >>= 1;

		posX += width;
		posY += height;

		// Get the list of monitors
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		if (monitors == nullptr)
			return;

		// Figure out which monitor the window is in
		GLFWmonitor* owner = NULL;
		int owner_x, owner_y, owner_width, owner_height;

		for (int i = 0; i < count; i++)
		{
			// Get the monitor position
			int monitor_x, monitor_y;
			glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

			// Get the monitor size from its video mode
			int monitor_width, monitor_height;
			GLFWvidmode* monitor_vidmode = (GLFWvidmode*)glfwGetVideoMode(monitors[i]);

			if (monitor_vidmode == NULL)
				continue;

			monitor_width = monitor_vidmode->width;
			monitor_height = monitor_vidmode->height;

			// Set the owner to this monitor if the center of the window is within its bounding box
			if ((posX > monitor_x && posX < (monitor_x + monitor_width)) && (posY > monitor_y && posY < (monitor_y + monitor_height)))
			{
				owner = monitors[i];
				owner_x = monitor_x;
				owner_y = monitor_y;
				owner_width = monitor_width;
				owner_height = monitor_height;
			}
		}

		// Set the window position to the center of the owner monitor
		if (owner != NULL)
			glfwSetWindowPos(_window, owner_x + (owner_width >> 1) - width, owner_y + (owner_height >> 1) - height);
	}
}