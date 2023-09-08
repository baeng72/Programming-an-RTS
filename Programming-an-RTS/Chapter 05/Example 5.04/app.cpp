#pragma once
#include <common.h>
#include "mouse.h"
#include "object.h"
#include "camera.h"
#include "functions.h"

class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	std::unique_ptr<Renderer::Font> _fontMouse;
	CAMERA _camera;
	MOUSE _mouse;
	int _intersectType;
	Renderer::DirectionalLight _light;
	std::vector<OBJECT> _gnomes;
	std::unique_ptr<Renderer::Line2D> _line;
	bool _areaSelect;
	INTPOINT _startSel;
	void Select(mat4&matProj,mat4&matView,vec4&viewport);
	int GetGnome(mat4& matProj, mat4& matView);
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
	_areaSelect = false;
	srand(123456);
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
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);


	_fontMouse.reset(Renderer::Font::Create());
	_fontMouse->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 24);

	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));

	_camera.Init(GetWindowPtr());

	LoadObjectResources(_device.get(), _shadermanager);

	_mouse.Init(_device.get(), GetWindowPtr());

	_line.reset(Renderer::Line2D::Create(_device.get()));
	_line->Update(width, height);

	_gnomes.push_back(OBJECT(GNOME, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "Sgt. Salty"));
	_gnomes.push_back(OBJECT(GNOME, vec3(2.0f, 0.0f, 0.0f), vec3(0.0f, 0.6f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "Gnomad"));
	_gnomes.push_back(OBJECT(GNOME, vec3(2.0f, 0.0f, 2.0f), vec3(0.0f, 2.6f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "Mr Finch"));
	_gnomes.push_back(OBJECT(GNOME, vec3(-3.0f, 0.0f, -4.0f), vec3(0.0f, 1.6f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "Hanz"));
	_gnomes.push_back(OBJECT(GNOME, vec3(-1.0f, 0.0f, 3.0f), vec3(0.0f, -0.6f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "von Wunderbaum"));
	_gnomes.push_back(OBJECT(GNOME, vec3(5.0f, 0.0f, -3.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "Iceman"));
	_gnomes.push_back(OBJECT(GNOME, vec3(-6.0f, 0.0f, 5.0f), vec3(0.0f, -1.6f, 0.0f), vec3(0.3f, 0.3f, 0.3f), "!!!"));


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

void APPLICATION::Select(mat4&matProj,mat4&matView,vec4&viewport) {
	if (_mouse.ClickLeft()) {
		
		// if the mouse button is pressed
		for (auto& gnome : _gnomes)//deselect all objects
			gnome._selected = false;

		if (!_areaSelect) {		//If no area selection is in progress
			//GetGnome() returns a Gnome using the mouse ray.
			//If no gnome is found this function returns -1.
			int gnome = GetGnome(matProj,matView);

			if (gnome >= 0)
				_gnomes[gnome]._selected = true;
			else {
				_areaSelect = true;	//if no gnome is found,
				_startSel = _mouse;	//start area selection
			}
		}
		else {	//Area Selection in progress
			mat4 matVP = matProj * matView;
			//Create area rectangle
			INTPOINT p1 = _startSel, p2 = _mouse;
			if (p1.x > p2.x)
				std::swap(p1.x, p2.x);
			if (p1.y > p2.y)
				std::swap(p1.y, p2.y);
			Rect selRect = { p1.x, p1.y, p2.x, p2.y };
			//Draw selection rectangle
			vec2 box[] = {
				vec2(p1.x,p1.y),vec2(p2.x,p1.y),
				vec2(p2.x,p2.y),vec2(p1.x,p2.y),
				vec2(p1.x,p1.y)
			};
			_line->Draw(box, 5, vec4(1.f));

			//Select any gnomes inside our rectangle
			for (int i = 0; i < _gnomes.size(); i++) {
				INTPOINT p = GetScreenPos(_gnomes[i].GetPosition(), matVP, viewport);
				if (p.inRect(selRect))
					_gnomes[i]._selected = true;
			}
		}
	}
	else if (_areaSelect)	//stop area selecting
		_areaSelect = false;
}

int APPLICATION::GetGnome(mat4 &matProj, mat4& matView) {
	//Find best Gnome
	int gnome = -1;
	mat4 matVP = matProj * matView;
	float bestDist = 100000.f;
	for (int i = 0; i < (int)_gnomes.size(); i++) {
		mat4 world = _gnomes[i]._meshInstance.GetWorldMatrix();
		RAY ray = _mouse.GetRay(matProj, matView, world);


		float dist = -1;//use different intersection types, not in original example...
		if (_intersectType == 0) {
			RAY ray = _mouse.GetRay(matProj, matView, _gnomes[i]._meshInstance.GetWorldMatrix());
			dist = ray.Intersect(_gnomes[i]._meshInstance);
		}
		else if (_intersectType == 1) {
			RAY ray = _mouse.GetRay(matProj, matView, glm::mat4(1.f));
			dist = ray.Intersect(_gnomes[i]._BBox);
		}
		else if (_intersectType == 2) {
			RAY ray = _mouse.GetRay(matProj, matView, glm::mat4(1.f));
			dist = ray.Intersect(_gnomes[i]._BSphere);
		}

		_gnomes[i].RenderBoundingVolume(_intersectType, matVP, _light);
		
		if (dist > 0.f && dist < bestDist) {
			gnome = i;
			bestDist = dist;
		}
	}
	return gnome;
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		
	glm::mat4 matView = _camera.GetViewMatrix();
	glm::mat4 matProj = _camera.GetProjectionMatrix();
	glm::mat4 viewProj = matProj * matView;
	vec4 viewport = { 0,0,_width,_height };
	for (size_t i = 0; i < _gnomes.size(); i++) {
		_gnomes[i].Render(viewProj, _light);
	}
	for (size_t i = 0; i < _gnomes.size(); i++) {
		_gnomes[i].PaintSelected(viewProj, viewport);
	}

	int gnome = GetGnome(matProj, matView);

	Select(matProj, matView, viewport);

	if (gnome != -1) {
		_fontMouse->Draw(_gnomes[gnome]._name.c_str(), _mouse.x + 2, _mouse.y + 24, Color(0.f, 0.f, 0.f, 1.f));
		_fontMouse->Draw(_gnomes[gnome]._name.c_str(), _mouse.x, _mouse.y + 22, Color(1.f, 1.f, 1.f, 1.f));
	}
	_fontMouse->Render();

	
	_font->Draw("Mouse Wheel: Change Camera Radius", 10, 10, Color(1.f));
	_font->Draw("Arrows: Change Camera Angle", 10, 30, Color(1.f));
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

int main() {
	APPLICATION app;
	if (app.Init(800, 600, "Example 5.4: RTS Unit Selection Example")) {
		app.Run();
	}
	return 0;
}
