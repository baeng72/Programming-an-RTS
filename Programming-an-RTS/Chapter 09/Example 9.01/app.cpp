#pragma once
#include <common.h>
#include "intpoint.h"
#include "mouse.h"
#include "camera.h"
#include "terrain.h"
#include "unit.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::shared_ptr<Core::ThreadPool> _threads;
	std::unique_ptr<Renderer::Font> _font;		
	std::unique_ptr<Renderer::Line2D> _line;
	Renderer::DirectionalLight _light;	
	TERRAIN _terrain;
	CAMERA _camera;
	MOUSE _mouse;
	std::vector<UNIT*> _units;
	bool _wireframe;
	bool _areaSelected;
	INTPOINT _startSelect;

	void AddUnits();
	void Select(mat4&matProj,mat4&matView);
	int GetUnit(mat4&matProj,mat4&matView);
	
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
	_areaSelected = false;
	srand(2);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;
	_threads.reset(new Core::ThreadPool());
	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));

	_line.reset(Renderer::Line2D::Create(_device.get()));
	_line->Update(_width, _height);
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(vec3(1.f, 0.6f, 0.5f));
	LoadObjectResources(_device.get(), _shadermanager);
	LoadMapObjectResources(_device.get());
	LoadUnitResources(_device.get(), _shadermanager);

	_terrain.Init(_device.get(),GetWindowPtr(), _shadermanager, INTPOINT(100, 100));

	_mouse.Init(_device.get(),_shadermanager, GetWindowPtr());

	_camera.Init(GetWindowPtr());
	_camera._focus = vec3(50, 10, -50);
	_camera._fov = 0.6f;
	_camera._radius = 50.f;

	AddUnits();
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_camera.Update(_mouse, _terrain, deltaTime);
	_mouse.Update(_terrain);
	for (auto unit : _units) {
		_threads->QueueJob([unit, deltaTime]() {unit->Update(deltaTime); });
		//unit->Update(deltaTime);
	}
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
		_terrain.GenerateRandomTerrain(GetWindowPtr(),9);	
		AddUnits();
	}
	else if (IsKeyPressed(KEY_F)) {
		_camera._focus = vec3(50, 10, -50);
	}
	else if (_mouse.ClickRight()) {
		float currTicks = _device->GetCurrentTicks();
		if (currTicks > 0.3f) {
			_mouse.DisableInput(300);
			//move units
			for (auto unit : _units) {
				if (unit->_selected)
					unit->Goto(_mouse._mappos);
			}			
		}
	}
	
	
}




void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	vec4 viewport{ 0,0,(float)_width,(float)_height };
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = _camera.GetViewMatrix();
	
	
	glm::mat4 matProj = _camera.GetProjectionMatrix();
	_camera.CalculateFrustum(matProj, matView);
	_mouse.CalculateMappos(matProj, matView, _terrain);
	glm::mat4 viewProj = matProj * matView;
	_device->StartRender();

	_terrain.Render(viewProj, matWorld, _light, _camera);

	
	
	_font->Draw("SPACE: Randomize Terrain", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	
	for (auto unit : _units) {
		if (!_camera.Cull(unit->GetBoundingBox())) {
			unit->Render(viewProj, _light);
			unit->PaintSelected(viewProj, viewport);
		}
	}



	
	
	Select(matProj,matView);
	
	
	_font->Render();
	_mouse.Paint(viewProj,_light);

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	UnloadUnitResources();
	UnloadMapObjectResources();
	UnloadObjectResources();
	
	_threads->Stop();
	for (auto unit : _units) {
		delete unit;
	}
	_units.clear();
	_terrain.Cleanup();
}

void APPLICATION::AddUnits() {
	for (auto& unit : _units) {
		delete unit;
	}
	_units.clear();

	//create random units
	for (int i = 0; i < 30; i++) {
		//Find random walkable position
		INTPOINT mp;
		bool ok = false;
		do {
			mp.Set(rand() % _terrain._size.x, rand() % _terrain._size.y);
			MAPTILE* tile = _terrain.GetTile(mp);
			if (tile)
				ok = tile->_walkable;
		} while (!ok);
		_units.push_back(new UNIT(rand() % 3, 0, mp, &_terrain, _device.get()));
	}
}

void APPLICATION::Select(mat4& matProj,mat4 &matView) {
	mat4 matVP = matProj * matView;
	vec4 viewport{ 0,0,(float)_width,(float)_height };
	
	if (_mouse.ClickLeft()) {
		//If the mouse button is pressed
		for (auto unit : _units) {
			unit->_selected = false;
		}

		if (!_areaSelected) {
			//If no area selection is in progress
			int unit = GetUnit(matProj,matView);

			if (unit > -1)
				_units[unit]->_selected = true;
			else {
				_areaSelected = true;			//if no unit found
				_startSelect = _mouse;		//start area selection
			}
		}
		else {
			//Area selection in progress
			INTPOINT p1 = _startSelect;
			INTPOINT p2 = _mouse;
			
			if (p1.x > p2.x) {
				std::swap(p1.x, p2.x);
			}
			if (p1.y > p2.y) {
				std::swap(p1.y, p2.y);
			}

			
			//Draw selection rectangle
			vec2 box[] = { vec2(p1.x, p1.y), vec2(p2.x, p1.y),
									 vec2(p2.x, p2.y), vec2(p1.x, p2.y),
									 vec2(p1.x, p1.y) };


			_line->Draw(box, 5, Color(1.f), 1.f);
		
			Rect selRect{ p1.x,p1.y,p2.x,p2.y };

			for (auto unit : _units) {
				INTPOINT p = GetScreenPos(matVP, viewport, unit->_position);
				if (p.inRect(selRect))
					unit->_selected = true;
			}

		}

	}
	else if(_areaSelected){
		_areaSelected = false;
		
	}
}

int APPLICATION::GetUnit(mat4& matProj,mat4&matView) {
	//find closest unit
	int unitid = -1;
	float bestDist = 1000000.f;

	RAY ray = _mouse.GetRay(matProj, matView, mat4(1.f));
	int i = 0;
	for (auto unit : _units) {
		float dist = ray.Intersect(unit->GetBoundingBox());

		if (dist >= 0.f && dist < bestDist) {
			unitid = i;
			bestDist = dist;
		}
		i++;
	}
	return unitid;
}



int main(int argc, char* argv[]) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(465433);
#endif
	if (argc > 1) {
		if (!_strcmpi(argv[1], "gl")) {

			Core::SetAPI(Core::API::GL);
		}
		else {
			Core::SetAPI(Core::API::Vulkan);
		}
	}
	APPLICATION app;
	if (app.Init(800, 600, "Example 9.1: Unit Example")) {
		app.Run(); 
	}
	return 0;
}
