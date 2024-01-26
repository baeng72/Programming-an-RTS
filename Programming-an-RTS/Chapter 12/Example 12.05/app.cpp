#include <common.h>

#include "heightMap.h"
#include "terrain.h"
#include "camera.h"
#include "mouse.h"
#include "skybox.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	TERRAIN _terrain;
	MOUSE _mouse;
	std::unique_ptr<SKYBOX> _skybox;	
	vec3 _rot;
	INTPOINT _lastM;
	bool _wireframe;	
	Renderer::DirectionalLight _light;
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
	srand(504511375);
	_rot = vec3(0.f);
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.05");

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
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	
	LoadObjectResources(_device.get());
	_terrain.Init(_device.get(), GetWindowPtr(), _shadermanager, INTPOINT(100,100));
	_mouse.Init(_device.get(), _shadermanager, GetWindowPtr());
	_lastM = _mouse;
	_skybox.reset(new SKYBOX(_device.get(),_shadermanager, Core::ResourcePath::GetTexturePath("skybox"), 500.f));
	_light.ambient = vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = vec4(1.f);
	_light.specular = vec4(0.1f, 0.1f, 0.1f, 1.f);
	_light.direction = vec3(1.f, 0.6f, 0.5f);
	_skybox->GenerateEnvironmentMap(_terrain.GetWorldPos(_terrain._size / 2) + vec3(0.f, 10.f, 0.f), true, _terrain,_light,GetWindowPtr());
	
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_mouse.Update(_terrain);

	if (_mouse != _lastM) {
		_rot.y -= (_lastM.x - _mouse.x) * 0.01f;
		_rot.z += (_lastM.y - _mouse.y) * 0.01f;
		_lastM = _mouse;
	}
	
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	else if (IsKeyPressed(VK_SPACE)) {
		_terrain.GenerateRandomTerrain(GetWindowPtr(),9);
		_skybox->GenerateEnvironmentMap(_terrain.GetWorldPos(_terrain._size / 2) + vec3(0.f, 10.f, 0.f), true, _terrain,_light,GetWindowPtr());
	}
	
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
	//_skybox->GenerateEnvironmentMap(_terrain.GetWorldPos(_terrain._size / 2) + vec3(0.f, 10.f, 0.f), true, _terrain, _light, GetWindowPtr());
	
	_skybox->Render(matVP,eye);
	_mouse.Paint(matVP, _light);


	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop(); 
	_device->Wait();
	UnloadObjectResources();
	_skybox.reset();
	_terrain.Cleanup();
	
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(185003);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 12.5: Environment Maps")) {
		app.Run();
	}

}