#pragma once
#include <common.h>
#include "intpoint.h"

#include "terrain.h"


class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;		
	Renderer::DirectionalLight _light;
	TERRAIN _terrain;
	
	float _angle;
	float _radius;
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
	_radius = 100.f;
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
	LoadObjectResources(_device.get(), _shadermanager);
	
	_terrain.Init(_device.get(), _shadermanager, INTPOINT(150, 150));
	
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_angle += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;		
		ObjectSetWireframe(_wireframe);
		_terrain.SetWireframe(_wireframe);
		
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		//Generate random terrain
		_terrain.GenerateRandomTerrain(3);
		Sleep(300);
	}
	else if (IsKeyPressed(KEY_KP_ADD) && _radius < 200.f) {
		//zoom out
		_radius += deltaTime * 30.f;
	}
	else if (IsKeyPressed(KEY_KP_SUBTRACT) && _radius > 5.f) {
		_radius -= deltaTime * 30.f;
	}
	
}


void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	glm::vec2 center = glm::vec2(_terrain._size.x / 2.0f, _terrain._size.y / 2.0f);
	glm::vec3 eye = glm::vec3(center.x + cos(_angle) * _radius,
		_radius,
		-center.y + sin(_angle) * _radius);
	glm::vec3 lookat = glm::vec3(center.x, 0.f, -center.y);
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = glm::lookAtLH(eye, lookat, up);
	
	constexpr float fov = glm::radians(45.f);	
	glm::mat4 matProj = glm::perspectiveFovLH_ZO(fov, (float)_width, (float)_height, 1.f, 1000.f);
	matProj[1][1] *= -1;//flip y for Vulkan should be in a specific class or something
		
	glm::mat4 viewProj = matProj * matView;

	
	
	_font->Draw("W: Toggle Wireframe", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("+/-: Zoom In/Out", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("SPACE: Randomize Terrain", 10, 50, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	char buffer[64];
	sprintf_s(buffer, "%d objects", (int)_terrain._objects.size());
	_font->Draw(buffer, 710, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));

	_device->StartRender();		

	_terrain.Render(viewProj, matWorld, _light);

	
	_font->Render();

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	UnloadObjectResources();
}


int main() {
	APPLICATION app;
	if (app.Init(800, 600, "Example 4.11: Placing Terrain Objects")) {
		app.Run();
	}
	return 0;
}
