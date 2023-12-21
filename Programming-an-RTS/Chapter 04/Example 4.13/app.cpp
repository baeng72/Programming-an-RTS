#pragma once
#include <common.h>
#include "intpoint.h"

#include "terrain.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::ParticleSwarm>	_particles;
	std::unique_ptr<Renderer::Font> _font;		
	Renderer::DirectionalLight _light;
	std::vector<Renderer::LineVertex> _path;
	std::unique_ptr<Renderer::Line> _line;
	TERRAIN _terrain;
	
	float _angle;
	float _radius;
	bool _wireframe;

	INTPOINT	_start;
	INTPOINT	_goal;
	
	
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
	Core::ResourcePath::SetProjectPath("Chapter 04/Example 4.13");
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));
	LoadObjectResources(_device.get(), _shadermanager);
	
	_terrain.Init(_device.get(), _shadermanager, INTPOINT(100, 100));

	
	_particles.reset(Renderer::ParticleSwarm::Create(_device.get(), nullptr,0, glm::vec2(0.5f)));
	
	_line.reset(Renderer::Line::Create(_device.get(), nullptr, 0));
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
		_path.clear();
		_particles->ResetVertices(nullptr, 0);
		_line->ResetVertices(nullptr, 0);
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_KP_ADD) && _radius < 200.f) {
		//zoom out
		_radius += deltaTime * 30.f;
	}
	else if (IsKeyPressed(KEY_KP_SUBTRACT) && _radius > 5.f) {
		_radius -= deltaTime * 30.f;
	}
	else if (IsKeyPressed(KEY_N)) {
		_device->Wait();
		do {
			_start = INTPOINT(rand() % _terrain._size.x, rand() % _terrain._size.y);
			_goal = INTPOINT(rand() % _terrain._size.x, rand() % _terrain._size.y);
			while (!_terrain.GetTile(_start)->_walkable) {
				_start = INTPOINT(rand() % _terrain._size.x, rand() % _terrain._size.y);
			}
			while (!_terrain.GetTile(_goal)->_walkable || _start == _goal) {
				_goal = INTPOINT(rand() % _terrain._size.x, rand() % _terrain._size.y);
			}
		} while (_terrain.GetTile(_start)->_set != _terrain.GetTile(_goal)->_set);

		std::vector<INTPOINT> p = _terrain.GetPath(_start, _goal);
		_path.clear();

		if (!p.empty()) {
			for (int i = 0; i < p.size(); i++) {
				float prc = i / (float)p.size();
				float red = prc;
				float green = (1 - prc);
				MAPTILE* t = _terrain.GetTile(p[i]);

				_path.push_back({ glm::vec3(p[i].x,t->_height +0.5f,-p[i].y),glm::vec4(red,green,0.f,1.f) });
			}
			_line->ResetVertices(_path.data(), (uint32_t)_path.size());
			std::vector<Renderer::ParticleVertex> particles;
			particles.push_back({ glm::vec3(_start.x, _terrain.GetTile(_start)->_height + 0.5f, -_start.y), glm::vec4(0.f, 1.f, 0.f, 1.f) });
			particles.push_back({ glm::vec3(_goal.x, _terrain.GetTile(_goal)->_height + 0.5f, -_goal.y), glm::vec4(1.f, 0.f, 0.f, 1.f) });
			for (int y = 0; y < _terrain._size.y; y++) {
				for (int x = 0; x < _terrain._size.x; x++) {
					if (_terrain.GetTile(x, y)->g < 1000.f) {

						particles.push_back({ glm::vec3(x,_terrain.GetTile(x,y)->_height + 0.1f,-y),glm::vec4(0.f,0.f,1.f,1.f) });
					}
				}
			}
			_particles->ResetVertices(particles.data(), (uint32_t)particles.size());
		}
		else {
			_line->ResetVertices(nullptr, 0);
			_particles->ResetVertices(nullptr, 0);
		}

		
		Sleep(200);
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
	glm::mat4 matProj = Core::perspective(quaterpi, (float)_width, (float)_height, 1.f, 1000.f);
	
		
	glm::mat4 viewProj = matProj * matView;

	
	
	_font->Draw("W: Toggle Wireframe", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("+/-: Zoom In/Out", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("N: New Path", 10, 50, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("SPACE: Randomize Terrain", 10, 70, glm::vec4(0.f, 0.f, 0.f, 1.f));

	if (_path.empty()) {
		_font->Draw("No Path Found!", 10, 110, glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	
	char buffer[64];
	sprintf_s(buffer, "%d objects", (int)_terrain._objects.size());
	_font->Draw(buffer, 710, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));

	_device->StartRender();		

	_terrain.Render(viewProj, matWorld, _light);
	_line->Draw(viewProj);
	_particles->Draw(viewProj, eye);
	_font->Render();

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	UnloadObjectResources();
	_terrain.Cleanup();
	_line.reset();
	_font.reset();
}


void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(171618);
#endif

	APPLICATION app;
	if (app.Init(800, 600, "Example 4.13: Pathfinding")) {
		app.Run();
	}
	
}
