#include <common.h>
#include "intpoint.h"
#include "Mesh.h"
#include <iostream>

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::shared_ptr<Renderer::Shader> _shader;
	std::unique_ptr<Renderer::Font> _font;	
	Renderer::DirectionalLight _light;
	MESH _build1;
	MESH _build2;
	float _buildPrc;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {	
	srand(693278906);	
	Core::ResourcePath::SetProjectPath("Chapter 12/Example 12.07");
	_buildPrc = 0.f;
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
	_shader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData(Core::ResourcePath::GetShaderPath("mesh.glsl"), Renderer::ShaderCullMode::backFace)));
	_build1.Load(_device.get(), Core::ResourcePath::GetMeshPath("townhall_b.x"));
	_build2.Load(_device.get(), Core::ResourcePath::GetMeshPath("townhall.x"));

	_light.ambient = vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = vec4(1.f);
	_light.specular = vec4(0.1f, 0.1f, 0.1f, 1.f);
	_light.direction = vec3(1.f, 0.6f, 0.5f);
	return true;
}

void APPLICATION::Update(float deltaTime) {
	
	//std::cout << "Rot: (" << _rot.x << "," << _rot.y << "," << _rot.z << ")" << std::endl;
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();

	_buildPrc += deltaTime * 0.07f;
	if (_buildPrc > 1.5f)
		_buildPrc = 0.f;
		
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
	
	//target
	mat4 view, proj, world, id;
	vec3 eye = vec3(15.f, 30.f, -40.f);
	vec3 center = vec3(0.f, 11.f, 0.f);
	vec3 up = vec3(0.f, 1.f, 0.f);
	view = glm::lookAtLH(eye, center, up);
	proj = Core::perspective(pi * 0.3f, (float)_width, (float)_height, 0.01f, 1000.f);
	mat4 viewProj = proj * view;
	id = mat4(1.f);
	_device->StartRender();
	_shader->SetUniform("viewProj", viewProj);
	
	_shader->SetUniform("light.ambient", &_light.ambient);
	_shader->SetUniform("light.diffuse", &_light.diffuse);
	_shader->SetUniform("light.specular", &_light.specular);
	_shader->SetUniform("light.direction", &_light.direction);
	_shader->Bind();
	if (_buildPrc < 1.f) {
		_shader->SetUniform("model", _build1.getXForm());
		_build1.Render(_shader.get(), _buildPrc);
	}
	else {
		_shader->SetUniform("model", _build2.getXForm());
		_build2.Render(_shader.get());
	}


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
	if (app.Init(800, 600, "Example 12.7: Constructing Buildings")) {
		app.Run();
	}

}