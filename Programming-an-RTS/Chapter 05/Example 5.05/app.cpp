#pragma once
#include <common.h>

#include "object.h"
#include "camera.h"


class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	
	Renderer::DirectionalLight _light;
	std::vector<OBJECT> _cars;
	std::unique_ptr<MESH> _track;
	
	Rect _destRects[2];
	Rect _srcRects[2];
	Rect _currentRects[2];

	int _viewConfig;
	float _viewPortPrc;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {

	_viewConfig = 0;
	_viewPortPrc = 0.0f;

	//Setup viewports
	_srcRects[0].left = 5;
	_srcRects[0].right = 395;
	_srcRects[0].top = 35;
	_srcRects[0].bottom = 595;

	_srcRects[1].left = 405;
	_srcRects[1].right = 795;
	_srcRects[1].top = 35;
	_srcRects[1].bottom = 595;

	_currentRects[0] = _destRects[0] = _srcRects[0];
	_currentRects[0] = _destRects[1] = _srcRects[1];
	
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


	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.5f, -0.6f, 0.f));

	

	LoadObjectResources(_device.get(), _shadermanager);

	
	_cars.push_back(OBJECT(GetWindowPtr(),0, vec3(0.f, 0.f, 0.5f), vec3(0.f), 1.6f));
	_cars.push_back(OBJECT(GetWindowPtr(),1, vec3(0.f, 0.f, -1.5f), vec3(0.f), -1.6f));
	_track = std::make_unique<MESH>(_device.get(), _shadermanager, "../../../../Resources/Chapter 05/Example 5.05/objects/track.x");


	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	for (size_t i = 0; i < _cars.size(); i++) {
		_cars[i].Update(deltaTime);
	}

	if (IsKeyPressed(KEY_F1)) {	//change camera 1
		_cars[0]._activeCam++;
		_cars[0]._activeCam %= _cars[0]._cameras.size();
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_F2)) { //change camera 1
		_cars[1]._activeCam++;
		_cars[1]._activeCam %= _cars[1]._cameras.size();
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE))		//Change Viewport settings
	{
		_viewConfig++;
		if (_viewConfig >= 3)
			_viewConfig = 0;

		if (_viewConfig == 0)		//Split X
		{
			_destRects[0].left = 5;
			_destRects[0].right = 395;
			_destRects[0].top = 35;
			_destRects[0].bottom = 595;
			_destRects[1].left = 405;
			_destRects[1].right = 795;
			_destRects[1].top = 35;
			_destRects[1].bottom = 595;
		}
		else if (_viewConfig == 1)		//Split Y
		{
			_destRects[0].left = 5;
			_destRects[0].right = 795;
			_destRects[0].top = 35;
			_destRects[0].bottom = 305;
			_destRects[1].left = 5;
			_destRects[1].right = 795;
			_destRects[1].top = 325;
			_destRects[1].bottom = 595;
		}
		else if (_viewConfig == 2)		//Big 1, Small 2
		{
			_destRects[0].left = 30;
			_destRects[0].right = 770;
			_destRects[0].top = 60;
			_destRects[0].bottom = 570;
			_destRects[1].left = 650;
			_destRects[1].right = 790;
			_destRects[1].top = 35;
			_destRects[1].bottom = 175;
		}
		_viewPortPrc = 0.0f;
		_srcRects[0] = _currentRects[0];
		_srcRects[1] = _currentRects[1];
		Sleep(300);
	}

	//Change viewport rectangles
	_viewPortPrc += deltaTime;
	if (_viewPortPrc > 1.0f)
		_viewPortPrc = 1.0f;

	//Linear interpolation between the two rectangles
	for (int i = 0; i < 2; i++)
	{
		_currentRects[i].left = _srcRects[i].left - (int)(_srcRects[i].left * _viewPortPrc) + (int)(_destRects[i].left * _viewPortPrc);
		_currentRects[i].right = _srcRects[i].right - (int)(_srcRects[i].right * _viewPortPrc) + (int)(_destRects[i].right * _viewPortPrc);
		_currentRects[i].top = _srcRects[i].top - (int)(_srcRects[i].top * _viewPortPrc) + (int)(_destRects[i].top * _viewPortPrc);
		_currentRects[i].bottom = _srcRects[i].bottom - (int)(_srcRects[i].bottom * _viewPortPrc) + (int)(_destRects[i].bottom * _viewPortPrc);
	}

	
}


void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	// Clear the viewport
	

	_device->StartRender();		

	Renderer::ViewPort v;
	v.x = v.y = 0;
	v.width = 800;
	v.height = 600;
	v.ffar = 1.0f;
	v.fnear = 0.0f;
	//_device->SetViewport(v);
	
	
	
	_font->Draw("Space: Viewport", 340, 10, Color(1.f));
	_font->Draw("F1: Camera 1", 40, 10, Color(1.f));
	_font->Draw("F2: Camera 2", 640, 10, Color(1.f));
	_font->Render();
	//For each car
	for (int c = 0; c < _cars.size(); c++)
	{
		//Set and clear viewport
		v.x =(float) _currentRects[c].left;
		v.y = (float)_currentRects[c].top;
		v.width = (float)(_currentRects[c].right - _currentRects[c].left);
		v.height = (float)(_currentRects[c].bottom - _currentRects[c].top);
		_device->SetViewport(v);
		_device->Clear(_currentRects[c], Color(44.f / 255.f, 44.f / 255.f, 1.f, 1.f));
		
		_cars[c].UpdateCameras();

		
		//Render track
		mat4 matVP = _cars[c]._matProj * _cars[c]._matView;
		_track->Render(matVP,glm::mat4(1.f),_light);

		//Render both cars
		for (int i = 0; i < _cars.size(); i++)
			_cars[i].Render(matVP,_light);
	}
	
	
	
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
