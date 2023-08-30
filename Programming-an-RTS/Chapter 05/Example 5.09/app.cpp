#pragma once
#include <common.h>
#include "intpoint.h"
#include "mouse.h"
#include "camera.h"
#include "terrain.h"


class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;		
	Renderer::DirectionalLight _light;	
	TERRAIN _terrain;
	CAMERA _camera;
	MOUSE _mouse;
	bool _wireframe;
	float _snapTime;
	
	
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
	_snapTime = 0.f;
	srand(11678390);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->SetVSync(true);
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
	
	_terrain.Init(_device.get(), _shadermanager, INTPOINT(100, 100));

	_mouse.Init(_device.get(),_shadermanager, GetWindowPtr());

	_camera.Init(GetWindowPtr());
	_camera._focus = vec3(50, 10, -50);
	_camera._fov = 0.6f;
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_camera.Update(_mouse, _terrain, deltaTime);
	_mouse.Update(_terrain);
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
	}
	else if (_mouse.ClickRight()) {
		float currTicks = _device->GetCurrentTicks();
		if (currTicks > 0.3f) {
			_snapTime = currTicks;
			_camera._focus = _terrain.GetWorldPos(_mouse._mappos);
		}
	}
	
	
}


void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = _camera.GetViewMatrix();
	
	
	glm::mat4 matProj = _camera.GetProjectionMatrix();
	_camera.CalculateFrustum(matProj, matView);
	_mouse.CalcualateMappos(matProj, matView, _terrain);
	glm::mat4 viewProj = matProj * matView;

	char buffer[64];
	sprintf_s(buffer, "Mouse Mappos: %d,%d", _mouse._mappos.x, _mouse._mappos.y);
	_font->Draw(buffer, 10, 10, Color(0.f, 0.f, 0.f, 1.f));
	_font->Draw("SPACE: Randomize Terrain", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("Right MButton ", 10, 50, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	
	
	_device->StartRender();		

	_terrain.Render(viewProj, matWorld, _light);

	_font->Render();
	_mouse.Paint(viewProj,_light);

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
	if (app.Init(800, 600, "Example 5.9: Second look at the Terrain")) {
		app.Run(); 
	}
	return 0;
}
