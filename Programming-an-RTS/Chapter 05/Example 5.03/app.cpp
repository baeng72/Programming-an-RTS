#pragma once
#include <common.h>
#include "mouse.h"
#include "object.h"
#include "camera.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	std::unique_ptr<Renderer::Font> _fontMouse;
	CAMERA _camera;
	MOUSE _mouse;
	int _intersectType;
	Renderer::DirectionalLight _light;
	std::vector<OBJECT> _objects;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	_intersectType = 0;
	Core::ResourcePath::SetProjectPath("Chapter 05/Example 5.03");
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->Init();
	//_device->SetClearColor(1.f, 1.f, 1.f, 1.f);

	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));


	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);


	_fontMouse.reset(Renderer::Font::Create());
	_fontMouse->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 24);

	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));

	_camera.Init(GetWindowPtr());

	LoadObjectResources(_device.get(), _shadermanager);

	_mouse.Init(_device.get(), GetWindowPtr());

	_objects.push_back(OBJECT(DRAGON, vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
	_objects.push_back(OBJECT(BOB, vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
	_objects.push_back(OBJECT(RING, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
	_mouse.x = 400;
	_mouse.y = 300;
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_mouse.Update();
	_camera.Update(_mouse,deltaTime);
	if (IsKeyPressed(KEY_SPACE)) {
		_intersectType++;
		_intersectType %= 3;
		Sleep(100);
	}
	
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		
	glm::mat4 matView = _camera.GetViewMatrix();
	glm::mat4 matProj = _camera.GetProjectionMatrix();
	glm::mat4 viewProj = matProj * matView;
	for (size_t i = 0; i < _objects.size(); i++) {
		_objects[i].Render(viewProj, _light);
	}
	int object = -1;
	float bestDist = 1000000.f;
	for (size_t i = 0; i < _objects.size() - 1; i++) {
		float dist = -1.f;

		//Do intersection tests
		if (_intersectType == 0) {
			RAY ray = _mouse.GetRay(matProj, matView, _objects[i]._meshInstance.GetWorldMatrix());
			dist = ray.Intersect(_objects[i]._meshInstance);
		}
		else if (_intersectType == 1) {
			RAY ray = _mouse.GetRay(matProj, matView, glm::mat4(1.f));
			dist = ray.Intersect(_objects[i]._BBox);
		}
		else if (_intersectType == 2) {
			RAY ray = _mouse.GetRay(matProj, matView, glm::mat4(1.f));
			dist = ray.Intersect(_objects[i]._BSphere);
		}

		_objects[i].RenderBoundingVolume(_intersectType,viewProj,_light);

		if (dist >= 0.f && dist < bestDist) {
			object = (int)i;
			bestDist = dist;
		}
	}
	if (object != -1) {
		_fontMouse->Draw(_objects[object]._name.c_str(), _mouse.x + 2, _mouse.y + 24, Color(0.f, 0.f, 0.f, 1.f));
		_fontMouse->Draw(_objects[object]._name.c_str(), _mouse.x, _mouse.y + 22, Color(1.f, 1.f, 1.f, 1.f));
	}
	_fontMouse->Render();

	
	_font->Draw("Mouse Wheel: Change Camera Radius", 10, 10, Color(1.f));
	_font->Draw("Arrows: Change Camera Angled", 10, 30, Color(1.f));
	_font->Draw("Space: Change Bounding Volumne", 10, 50, Color(1.f));
	if (_intersectType == 0) {
		_font->Draw("Mesh Intersection Test", 500, 10, Color(1.f));
	}
	else if (_intersectType == 1) {
		_font->Draw("Box Intersection Test", 500, 10, Color(1.f));
	}
	else if (_intersectType == 2) {
		_font->Draw("Sphere Intersection Test", 500, 10, Color(1.f));
	}
	_font->Render();
	_mouse.Paint();
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
	
}

void APPLICATION::Cleanup() {
	_device->Wait();
	UnloadObjectResources();
}

void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	APPLICATION app;
	if (app.Init(800, 600, "Example 5.3: Brave Bob vs. Dragon (a.k.a. Picking Example)")) {
		app.Run();
	}
	
}
