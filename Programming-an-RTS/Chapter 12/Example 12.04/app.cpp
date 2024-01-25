#include <common.h>
#include "intpoint.h"
#include "skybox.h"
#include "mouse.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	std::unique_ptr<SKYBOX> _skybox;
	MOUSE _mouse;
	vec3 _rot;
	INTPOINT _lastM;
	bool _wireframe;
	
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	_wireframe = false;
	srand(693278906);
	_rot = vec3(0.f);
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.04");

}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init() thread id: {0}", std::this_thread::get_id());
	if (!Application::Init(width, height, title))
		return false;

	_threads = std::make_unique<Core::ThreadPool>(8);
	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(0.f, 0.f, 0.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	_mouse.Init(_device.get(), _shadermanager, GetWindowPtr());
	
	_skybox.reset(new SKYBOX(_device.get(),_shadermanager, Core::ResourcePath::GetTexturePath("skybox"), 300.f));
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_mouse.Update();

	if (_mouse != _lastM) {
		_rot.y -= (_lastM.x - _mouse.x) * 0.01f;
		_rot.z += (_lastM.y - _mouse.y) * 0.01f;
		_lastM = _mouse;
	}
	
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	
	
}

void APPLICATION::Render() {
	mat4 view, proj, world;
	quat q = quat(_rot);
	mat4 m = mat4(q);
	vec3 focus = vec3(1.f, 0.f, 0.f);
	focus = vec3(m * vec4(1.f, 0.f, 0.f,0.f));
	vec3 eye = vec3(0.f);
	
	vec3 up = vec3(0.f, 1.f, 0.f);
	view = glm::lookAtLH(eye, focus, up);
	proj = Core::perspective(0.4f, 800, 600, 0.01f, 1000.f);
	mat4 matVP = proj * view;
	
	world = mat4(1.f);
	_device->StartRender();
	//target
	
	_skybox->Render(matVP,eye);
	


	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop(); 
	_device->Wait();
	
	
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(185003);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 12.4: Skybox Example")) {
		app.Run();
	}

}