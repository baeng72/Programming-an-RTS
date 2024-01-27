#include <common.h>
#include "intpoint.h"
#include "skybox.h"
#include "effect.h"
#include "mouse.h"
#include <iostream>

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
	std::vector<EFFECT*> _effects;
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
	srand(693278906);
	_rot = vec3(0.f);
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.06");

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
	_lastM = _mouse;
	//_lastM.y -= 100;

	_skybox.reset(new SKYBOX(_device.get(),_shadermanager, Core::ResourcePath::GetTexturePath("skybox"), 300.f));
	LoadEffectResources(_device.get(), _shadermanager);
	_effects.push_back(new EFFECT_LENSFLARE(_device.get(), 0, vec3(-200.f, 290.f, 0.f)));
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_mouse.Update();
	
	//remove dead effects
	std::vector<EFFECT*> toDelete;
	for (auto effect : _effects) {
		if (effect->isDead())
			toDelete.push_back(effect);
		else {
			effect->Update(deltaTime);
		}
	}
	for (auto effect : toDelete) {
		_effects.erase(std::find(_effects.begin(), _effects.end(), effect));
		delete effect;
	}

	if (_mouse != _lastM) {
		_rot.y -= (_lastM.x - _mouse.x) * 0.1f;
		_rot.z += (_lastM.y - _mouse.y) * 0.1f;
		
		_lastM = _mouse;
		//std::cout << "Mouse pos: " << _mouse.x << ", " << _mouse.y << ". Rot: (" << _rot.x << "," << _rot.y << "," << _rot.z << ")" << std::endl;
	}
	if (IsKeyPressed(KEY_UP)) {
		_rot.z += 0.001f;
	}
	else if (IsKeyPressed(KEY_DOWN)) {
		_rot.z -= 0.001f;
	}
	else if (IsKeyPressed(KEY_LEFT)) {
		_rot.y += 0.001f;
	}
	else if (IsKeyPressed(KEY_RIGHT)) {
		_rot.y -= 0.001f;
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		_rot.y = _rot.z = 0.f;
		
	}
	//std::cout << "Rot: (" << _rot.x << "," << _rot.y << "," << _rot.z << ")" << std::endl;
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	
	
}

inline mat4 RotationYawPitchRoll(float yaw, float pitch, float roll) {
	mat4 m(1.f);
	float sroll = sinf(roll);
	float croll = cosf(roll);
	float spitch = sinf(pitch);
	float cpitch = cosf(pitch);
	float syaw = sinf(yaw);
	float cyaw = cosf(yaw);
	m[0][0] = sroll * spitch * syaw + croll * cyaw;
	m[0][1] = sroll * cpitch;
	m[0][2] = sroll * spitch * cyaw - croll * syaw;
	
	m[1][0] = croll * spitch * syaw - sroll * cyaw;
	m[1][1] = croll * cpitch;
	m[1][2] = croll * spitch * cyaw + sroll * syaw;
	
	m[2][0] = cpitch * syaw;
	m[2][1] = -spitch;
	m[2][2] = cpitch * cyaw;

	m[3][3] = 0.f;

	return m;
}

inline mat4 RotationYawPitchRoll(vec3& rot) {
	mat4 m(1.f);
	float sroll = sinf(rot.z);
	float croll = cosf(rot.z);
	float spitch = sinf(rot.y);
	float cpitch = cosf(rot.y);
	float syaw = sinf(rot.x);
	float cyaw = cosf(rot.x);
	m[0][0] = sroll * spitch * syaw + croll * cyaw;
	m[0][1] = sroll * cpitch;
	m[0][2] = sroll * spitch * cyaw - croll * syaw;

	m[1][0] = croll * spitch * syaw - sroll * cyaw;
	m[1][1] = croll * cpitch;
	m[1][2] = croll * spitch * cyaw + sroll * syaw;

	m[2][0] = cpitch * syaw;
	m[2][1] = -spitch;
	m[2][2] = cpitch * cyaw;

	m[3][3] = 0.f;

	return m;
}
void APPLICATION::Render() {
	mat4 view, proj, world;
	quat q = quat(_rot);
	mat4 m = mat4(q);
	//mat4 m = RotationYawPitchRoll(_rot);
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
	
	for (auto effect : _effects) {
		effect->Render(matVP);
	}

	//_mouse.Paint(matVP, _light);


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
	UnloadEffectResources();
	
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(185003);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 12.6: Lens Flare Effect")) {
		app.Run();
	}

}