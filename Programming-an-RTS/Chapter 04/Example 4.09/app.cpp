#pragma once
#include <common.h>

#include "Mesh.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;		
	Renderer::DirectionalLight _light;
	MESH _mesh1, _mesh2;
	int  _activeMesh;
	float _angle;		
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
	_angle = 0.f;	
	_wireframe = false;
	_activeMesh = 0;
	srand(411678390);
	Core::ResourcePath::SetProjectPath("Chapter 04/Example 4.09");
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));

	if (!_mesh1.Load(_device.get(),_shadermanager, Core::ResourcePath::GetMeshPath("tree.x"))) {
		LOG_ERROR("Unable to load tree.x!");
	}
	if (!_mesh2.Load(_device.get(),_shadermanager, Core::ResourcePath::GetMeshPath("stone.x"))) {
		LOG_ERROR("Unable to load stong.x!");
	}
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_angle += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;		
		_mesh1.SetWireframe(_wireframe);
		_mesh2.SetWireframe(_wireframe);
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		_activeMesh++;
		if (_activeMesh >= 2)
			_activeMesh = 0;		
		Sleep(300);
	}
	
}


void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.

	
	glm::vec3 eye = glm::vec3( cos(_angle) * 2.f,	1.5f,	sin(_angle) * 2.f);
	glm::vec3 lookat = glm::vec3(0.f,0.5f,0.f);
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = glm::lookAtLH(eye, lookat, up);
	glm::mat4 matProj = Core::perspective(quaterpi, (float)_width, (float)_height, 1.f, 1000.f);
	
		
	glm::mat4 viewProj = matProj * matView;

	
	
	_font->Draw("SPACE: Next Object", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("W: Toggle Wireframe", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	

	_device->StartRender();		

	if (_activeMesh == 0)
		_mesh1.Render(viewProj,matWorld,_light);
	else
		_mesh2.Render(viewProj,matWorld,_light);
	_font->Render();

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	APPLICATION app;
	if (app.Init(800, 600, "Example 4.9: Loading & Rendering X-files")) {
		app.Run();
	}
	
}
