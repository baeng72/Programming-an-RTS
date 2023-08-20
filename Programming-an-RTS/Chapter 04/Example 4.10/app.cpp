#pragma once
#include <common.h>

#include "Mesh.h"


class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;		
	Renderer::DirectionalLight _light;
	MESH _mesh1, _mesh2;
	std::vector<MESHINSTANCE> _meshInstances;
	
	float _angle;		
	bool _wireframe;
	
	void CreateMeshInstances();
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
	
	srand(411678390);
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
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));

	if (!_mesh1.Load(_device.get(),_shadermanager, "../../../../Resources/Chapter 04/Example 4.09/meshes/tree.x")) {
		LOG_ERROR("Unable to load tree.x!");
	}
	if (!_mesh2.Load(_device.get(),_shadermanager, "../../../../Resources/Chapter 04/Example 4.09/meshes/stone.x")) {
		LOG_ERROR("Unable to load stong.x!");
	}
	
	CreateMeshInstances();
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
		CreateMeshInstances();
			
		Sleep(300);
	}
	
}


void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.

	
	glm::vec3 eye = glm::vec3( cos(_angle) * 8.f,	5.5f,	sin(_angle) * 8.f);
	glm::vec3 lookat = glm::vec3(0.f,0.0f,0.f);
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = glm::lookAtLH(eye, lookat, up);
	
	constexpr float fov = glm::radians(45.f);	
	glm::mat4 matProj = glm::perspectiveFovLH_ZO(fov, (float)_width, (float)_height, 1.f, 1000.f);
	matProj[1][1] *= -1;//flip y for Vulkan should be in a specific class or something
		
	glm::mat4 viewProj = matProj * matView;

	
	
	_font->Draw("SPACE: Next Object", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("W: Toggle Wireframe", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	

	_device->StartRender();		

	for (auto meshinst : _meshInstances) {
		meshinst.Render(viewProj, _light);
	}
	_font->Render();

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

void APPLICATION::CreateMeshInstances() {
	_meshInstances.clear();

	//create a new intance with random type, rotation & scale
	for (int y = -5; y <= 5; y++) {
		for (int x = -5; x <= 5; x++) {
			MESHINSTANCE m;
			if (rand() % 2 == 0)
				m.SetMesh(&_mesh1);
			else
				m.SetMesh(&_mesh2);

			m.SetPosition(glm::vec3(x, 0.f, y));

			float rX = glm::pi<float>() * ((rand() % 50) / 1000.f);
			float rY = glm::pi<float>() * ((rand() % 1000) / 1000.f);
			float rZ = glm::pi<float>() * ((rand() % 50) / 1000.f);
			m.SetRotation(glm::vec3(rX, rY, rZ));

			float scale = (rand() % 750 + 250) / 1000.f;
			m.SetScale(glm::vec3(scale));

			_meshInstances.push_back(m);
		}
	}
}

int main() {
	APPLICATION app;
	if (app.Init(800, 600, "Example 4.10: Mesh Instances")) {
		app.Run();
	}
	return 0;
}
