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
		_mousebuttons[0] = _mousebuttons[1] = _mousebuttons[2] = 0;
	}

	Application::~Application()
	{
		
	}

	bool Application::Init(int width, int height, const char* title) {
		_width = width;
		_height = height;
		_title = title;
		_window = std::unique_ptr<Window>(Window::Create(width, height, title));
		_mousepos = vec2(width * 0.5f, height * 0.5f);
		_window->SetEventHandler(std::bind(&Application::OnEvent, this, std::placeholders::_1));
		return true;
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(EventType::WindowClose, std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowResizeEvent>(EventType::WindowResize, std::bind(&Application::OnWindowResize, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyPressedEvent>(EventType::KeyPressed, std::bind(&Application::OnKeyPressed, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyReleasedEvent>(EventType::KeyReleased, std::bind(&Application::OnKeyReleased, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonPressedEvent>(EventType::MouseButtonPressed, std::bind(&Application::OnMouseButtonPressed, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(EventType::MouseButtonReleased, std::bind(&Application::OnMouseButtonReleased, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseScrolledEvent>(EventType::MouseScrolled, std::bind(&Application::OnMouseScrolled, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseMovedEvent>(EventType::MouseMoved, std::bind(&Application::OnMouseMoved, this, std::placeholders::_1));
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
		_window->Shutdown();
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

	bool Application::OnKeyPressed(KeyPressedEvent& ev) {
		_keys.insert(ev.key);
		return true;
	}

	bool Application::OnKeyReleased(KeyReleasedEvent& ev) {
		_keys.erase(ev.key);
		return true;
	}

	bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& ev) {
		_mousebuttons[ev.button] = 1;
		return true;
	}

	bool Application::OnMouseButtonReleased(MouseButtonReleasedEvent& ev) {
		_mousebuttons[ev.button] = 0;
		return true;
	}

	bool Application::OnMouseScrolled(MouseScrolledEvent& ev) {
		_mousescroll[0] = ev.xOffset;
		_mousescroll[1] = ev.yOffset;
		return true;
	}

	bool Application::OnMouseMoved(MouseMovedEvent& ev) {
		_mousepos[0] = ev.xPos;
		_mousepos[1] = ev.yPos;
		return true;
	}
}
