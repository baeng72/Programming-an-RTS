#include <common.h>
#include "intpoint.h"
#include "skinnedMesh.h"
#include "effect.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	std::unique_ptr<SKINNEDMESH> _skinnedMesh;
	
	std::vector<EFFECT*> _effects;
	bool _wireframe;
	bool _showMagician;
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
	_showMagician = true;
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.01");

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
	
	_skinnedMesh = std::make_unique<SKINNEDMESH>();
	_skinnedMesh->Load(_device.get(),_shadermanager,Core::ResourcePath::GetProjectResourcePath("units/magician.x"));
	_skinnedMesh->Update();
	_skinnedMesh->SetAnimation("Still");
	_skinnedMesh->SetPose(0.f);
	
	LoadEffectResources(_device.get(), _shadermanager);
	return true;
}

void APPLICATION::Update(float deltaTime) {
	//remove dead effects
	std::vector<EFFECT*> todelete;
	for (auto& effect : _effects) {
		if (effect->isDead())
			todelete.push_back(effect);
		else
			effect->Update(deltaTime);
	}
	for (auto& effect : todelete) {
		_effects.erase(std::find(_effects.begin(), _effects.end(), effect));
		delete effect;
	}
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;

	

		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		if (_effects.empty())
			_effects.push_back(new EFFECT_SPELL(_device.get(), vec3(0)));
	}
}

void APPLICATION::Render() {
	mat4 view, proj, world;
	vec3 eye = vec3(0.f, 20.f, -50.f);
	vec3 center = vec3(0.f, 3.f, 0.f);
	vec3 up = vec3(0.f, 1.f, 0.f);
	view = glm::lookAtLH(eye, center, up);
	proj = Core::perspective(0.2f, 800, 600, 0.01f, 1000.f);
	mat4 matVP = proj * view;
	
	world = mat4(1.f);
	_device->StartRender();
	if (_showMagician) {
		auto shader = _skinnedMesh->GetShader();
		shader->SetUniform("viewProj", matVP);
		shader->SetUniform("model", _skinnedMesh->GetWorldXForm());
		shader->SetUniform("light.ambient", vec4(0.1f));
		shader->SetUniform("light.diffuse", vec4(1.f));
		shader->SetUniform("light.specular", vec4(0.01f));
		shader->SetUniform("light.direction", vec3(0.5f, 1.f, -0.5f));
		shader->SetUniform("color", vec4(1.f, 0.f, 0.f, 1.f));
		
		_skinnedMesh->Render(shader);
	}
	//Render effects
	for (int i = 0; i < _effects.size(); i++) {
		if (_effects[i]) {
			_effects[i]->Render(matVP);
		}
	}
	_font->Draw("Space: Cast Spell", 10, 10);
	_font->Draw("Return: Show Magician On/Off", 10, 30);
	_font->Render();
	


	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop();
	UnloadEffectResources();
	_device->Wait();
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(465433);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 12.1: Billboard Effect")) {
		app.Run();
	}

}