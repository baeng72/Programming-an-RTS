#include "Window.h"
#include "Api.h"
#include "../Platform/glfw/GLFW_Window_GL.h"
#include "../Platform/glfw/GLFW_Window_Vulk.h"

namespace Core {
	Window* Window::Create(uint32_t width, uint32_t height, const std::string& title) {
		switch (GetAPI()) {
		case API::GL:
			return new GLFW::GLFW_Window_GL(width, height, title);
		case API::Vulkan:
			return new GLFW::GLFW_Window_Vulk(width, height, title);
		}
		//return std::make_shared<GLFW_Window_Vulk>(width, height, title);
		assert(0);
		return nullptr;
	}
}