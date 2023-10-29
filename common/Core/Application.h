#pragma once
#include "Window.h"
namespace Core {
	class Application {
		static Application* _instance;
		std::unique_ptr<Window> _window;

		bool _running{ true };
		bool _minimized{ false };
		float _lastFrameTime{ 0.f };
		bool OnWindowClose(WindowCloseEvent& ev);
		bool OnWindowResize(WindowResizeEvent& ev);
	protected:
		std::string _title;
		int _width;
		int _height;
		int _fps;
		int _lastFps;
		float _time;
		void SetRunning(bool running) { _running = running; }
		bool IsRunning()const { return _running; }
		bool IsMinimized()const { return _minimized; }
		bool IsKeyPressed(int key)const { return _window->IsKeyPressed(key); }
		int GetFPS()const { return _lastFps; }
	public:
		Application();
		virtual ~Application();
		void OnEvent(Event& e);
		inline Window& GetWindow() { return *_window; }
		inline Window* GetWindowPtr() { return _window.get(); }
		inline static Application& Get() { return *_instance; }
		void Run();
		//mimic app functions from DX9 code
		virtual bool Init(int width, int height, const char* title);
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void Cleanup() = 0;
		virtual void Quit() = 0;
		
	};

	Application* CreateApplication();
}