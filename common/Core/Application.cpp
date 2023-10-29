#include "Application.h"
#include "Log.h"
#include <cassert>
#include <GLFW/glfw3.h>
namespace Core {
	Application* Application::_instance = nullptr;

	Application::Application()
	{
		ASSERT(!_instance, "Only 1 application instance allowed!");//only 1 instance
		_instance = this;
		
		_fps = _lastFps = 0;
	}

	Application::~Application()
	{
		
	}

	bool Application::Init(int width, int height, const char* title) {
		_width = width;
		_height = height;
		_title = title;
		_window = std::unique_ptr<Window>(Window::Create(width, height, title));
		_window->SetEventHandler(std::bind(&Application::OnEvent, this, std::placeholders::_1));
		return true;
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(EventType::WindowClose, std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowResizeEvent>(EventType::WindowResize, std::bind(&Application::OnWindowResize, this, std::placeholders::_1));

	}

	void Application::Run()
	{
		_time = (float)glfwGetTime();
		while (_running) {
			float time = (float)glfwGetTime();
			float delta = time - _lastFrameTime;
			_lastFrameTime = time;
			_window->OnBeginUpdate();

			_window->OnUpdate();

			if (!_minimized) {

				Update(delta);
				Render();

			}
			_fps++;
			if (time - _time > 1.f) {
				_lastFps = _fps;
				_fps = 0;
				_time = time;
				char buffer[128];
				sprintf_s(buffer, "%s: FPS %d", _title.c_str(),_lastFps);
				_window->SetTitle(buffer);

			}
		}
		Cleanup();
	}



	bool Application::OnWindowClose(WindowCloseEvent& e) {
		_running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		if (e.width == 0 || e.height == 0) {
			_minimized = true;
			return false;
		}
		_minimized = false;
		return false;
	}
}