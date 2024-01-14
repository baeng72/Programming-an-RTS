#include <common.h>
#include "intpoint.h"
#include "effect.h"
#include "particles.h"
#include "Mesh.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	MESH _floor;
	std::unique_ptr<Renderer::Shader> _meshShader;
	std::vector<EFFECT*> _effects;
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
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.03");

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
	
	//_skinnedMesh = std::make_unique<SKINNEDMESH>();
	//_skinnedMesh->Load(_device.get(),_shadermanager,Core::ResourcePath::GetProjectResourcePath("units/magician.x"));
	//_skinnedMesh->Update();
	//_skinnedMesh->SetAnimation("Still");
	//_skinnedMesh->SetPose(0.f);

	//_target.Load(_device.get(), Core::ResourcePath::GetMeshPath("target.x"));
	//_targetInstance.SetMesh(&_target);
	////Position objects
	//_targetInstance.SetPosition(vec3(15.f, 0.f, 40.f));
	//_targetInstance.SetRotation(vec3(0.5f, 0.6f, 0.f));
	//_targetInstance.SetScale(vec3(0.7f));
	_meshShader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData(Core::ResourcePath::GetShaderPath("mesh.glsl"))));
	
	
	_floor.Load(_device.get(), Core::ResourcePath::GetMeshPath("floor.x"));
	//_magicianTrans.Init(vec3(-5.f, 0.f, -10.f), vec3(0.f, pi + 0.4f, 0.f), vec3(1.f));
	//
	LoadEffectResources(_device.get(), _shadermanager);
	LoadParticleResources(_device.get(), _shadermanager);
	_effects.push_back(new MAGIC_SHOWER(_device.get(), 1000, vec3(0.f, 9.f, 0.f)));
	_effects.push_back(new MAGIC_SHOWER(_device.get(), 500, vec3(10.f, 7.f, 0.f)));
	_effects.push_back(new MAGIC_SHOWER(_device.get(), 500, vec3(-10.f, 7.f, 0.f)));
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
	
	
}

void APPLICATION::Render() {
	mat4 view, proj, world;
	vec3 eye = vec3(0.f, 20.f, -50.f);
	vec3 center = vec3(0.f, 3.f, 0.f);
	vec3 up = vec3(0.f, 1.f, 0.f);
	view = glm::lookAtLH(eye, center, up);
	proj = Core::perspective(0.4f, 800, 600, 0.01f, 1000.f);
	mat4 matVP = proj * view;
	
	world = mat4(1.f);
	_device->StartRender();
	//target
	_meshShader->Bind();
	_meshShader->SetUniform("viewProj", &matVP);
	_meshShader->SetUniform("light.ambient", vec4(0.5f));
	_meshShader->SetUniform("light.diffuse", vec4(1.f));
	_meshShader->SetUniform("light.specular", vec4(0.01f));
	_meshShader->SetUniform("light.direction", glm::normalize(vec3(0.5f, 1.f, -0.5f)));
	_meshShader->SetUniform("model", mat4(1.f));
	//Render effects
	_floor.Render(_meshShader.get());
	for (int i = 0; i < _effects.size(); i++) {
		if (_effects[i]) {
			_effects[i]->Render(matVP);
		}
	}
	
	


	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop(); 
	_device->Wait();
	for (auto effect : _effects) {
		delete effect;
	}
	_effects.clear();
	UnloadParticleResources();
	UnloadEffectResources();
	
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(185003);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 12.3: Particle Systems")) {
		app.Run();
	}

}