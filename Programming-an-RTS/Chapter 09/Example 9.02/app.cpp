#pragma once
#include <common.h>
#include "intpoint.h"
#include "mouse.h"
#include "camera.h"
#include "terrain.h"
#include "building.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::shared_ptr<Renderer::Shader> _buildingShader;
	std::unique_ptr<Renderer::Font> _font;		
	std::unique_ptr<Renderer::Line2D> _line;
	Renderer::DirectionalLight _light;	
	TERRAIN _terrain;
	CAMERA _camera;
	MOUSE _mouse;
	std::vector<BUILDING*> _buildings;
	BUILDING* _pBuildToPlace;
	bool _wireframe;
	bool _placeBuilding;
	int _placeType;

	void Select(mat4& matProj, mat4& matView);
	int GetBuilding(mat4& matProj, mat4& matView);
	
	
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
	_placeBuilding = false;
	_pBuildToPlace = nullptr;
	_placeType = 0;
	srand(223322);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	//_device->EnableGeometry(true);
	_device->SetVSync(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	if (Core::GetAPI() == Core::API::Vulkan) {
		_buildingShader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.02/shaders/Vulkan/building.glsl")));
	}
	else {
		_buildingShader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.02/shaders/GL/building.glsl")));
	}
	_line.reset(Renderer::Line2D::Create(_device.get()));
	_line->Update(_width, _height);
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(vec3(.5f, 1.f, -0.5f));
	LoadObjectResources(_device.get());
	LoadMapObjectResources(_device.get());
	LoadBuildingResources(_device.get());

	_terrain.Init(_device.get(),GetWindowPtr(), _shadermanager, INTPOINT(100, 100));

	_mouse.Init(_device.get(),_shadermanager, GetWindowPtr());

	_camera.Init(GetWindowPtr());
	_camera._focus = vec3(50, 10, -50);
	_camera._fov = 0.6f;
	_camera._radius = 50.f;

	
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_camera.Update(_mouse, _terrain, deltaTime);
	_mouse.Update(_terrain);
	for (auto unit : _buildings) {
		unit->Update(deltaTime);
	}
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;		
		//ObjectSetWireframe(_wireframe);
		_terrain.SetWireframe(_wireframe);
		
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		//Generate random terrain		
		_terrain.GenerateRandomTerrain(GetWindowPtr(),9);	
		for (auto building : _buildings) {
			delete building;
		}
		_buildings.clear();
	}
	else if (IsKeyPressed('1')) {
		_placeType = 0;
		_placeBuilding = true;
	}
	else if (IsKeyPressed('2')) {
		_placeType = 1;
		_placeBuilding = true;
	}
	else if (IsKeyPressed('3')) {
		_placeType = 2;
		_placeBuilding = true;
	}
	else if (IsKeyPressed(KEY_F)) {
		_camera._focus = vec3(50, 10, -50);
	}
	//Create building to place...
	if (_pBuildToPlace)
		delete _pBuildToPlace;
	_pBuildToPlace = nullptr;

	if (_placeBuilding)
		_pBuildToPlace = new BUILDING(_placeType, _mouse._mappos, &_terrain,false);

	//Mouse input
	if (_mouse.ClickLeft() && _placeBuilding) {
		_mouse.DisableInput(300);

		if (PlaceOk(_placeType, _mouse._mappos, &_terrain)) {
			_buildings.push_back(new BUILDING(_placeType, _mouse._mappos, &_terrain, true));
			_placeBuilding = false;
		}
	}
	else if (_mouse.ClickRight() && _placeBuilding)
		_placeBuilding = false;
	
	
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
	vec4 teamColor = vec4(1.f);
	
	vec4 color = vec4(1.f);
	
	_terrain.Render(viewProj, matWorld, _light, _camera);
	_buildingShader->Bind();
	vec2 mapSize(100.f);
	
	_buildingShader->SetUniform("viewProj", &viewProj);
	_buildingShader->SetUniform("light.ambient", &_light.ambient);
	_buildingShader->SetUniform("light.diffuse", &_light.diffuse);
	_buildingShader->SetUniform("light.specular", &_light.specular);
	_buildingShader->SetUniform("light.direction", &_light.direction);
	_buildingShader->SetUniform("mapSize", &mapSize);
	_buildingShader->SetUniform("color", &color);
	Renderer::Texture* plight = _terrain.GetLightMap();
	_buildingShader->SetTexture("lightmap", &plight, 1);
		
	

	for (auto building : _buildings) {
		if (building && !_camera.Cull(building->GetBoundingBox())) {			
			_buildingShader->SetUniform("teamColor", &teamColor);			
			building->Render(_buildingShader.get());
		}
	}

	//Render building to place
	if (_placeBuilding) {
	
		
		teamColor = (vec4(1.f, 0.f, 0.f, 1.f));
		if (PlaceOk(_placeType, _mouse._mappos, &_terrain)) {

			teamColor = vec4(0.f, 1.f, 0.f, 1.f);
		}
		_buildingShader->SetUniform("teamColor", &teamColor);			
		_pBuildToPlace->Render(_buildingShader.get());

		
	}
	for (auto building : _buildings) {
		if (building && !_camera.Cull(building->GetBoundingBox())) {
			building->PaintSelected(viewProj,viewport);
		}
	}
	
	
	
	Select(matProj,matView);

	_font->Draw("SPACE: Randomize Terrain", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("1: Townhall", 10, 50, vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("2: Barracks", 10, 70, vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("3: Tower", 10, 90, vec4(0.f, 0.f, 0.f, 1.f));
	
	_font->Render();
	_mouse.Paint(viewProj,_light);

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	UnloadBuildingResources();
	UnloadMapObjectResources();
	UnloadObjectResources();
	_terrain.Cleanup();
}



void APPLICATION::Select(mat4& matProj,mat4 &matView) {
	mat4 matVP = matProj * matView;
	vec4 viewport{ 0,0,(float)_width,(float)_height };
	if (_mouse.ClickLeft()) {
		//If the mouse button is pressed
		for (auto building : _buildings) {
			building->_selected = false;
		}

		
		int building = GetBuilding(matProj, matView);
		if (building > -1)
			_buildings[building]->_selected = true;

	}
	
}

int APPLICATION::GetBuilding(mat4& matProj, mat4& matView) {
	int build = -1;
	float bestDist = 100000.f;
	RAY ray = _mouse.GetRay(matProj, matView, mat4(1.f));

	for (size_t i = 0; i < _buildings.size(); i++) {
		float dist = ray.Intersect(_buildings[i]->GetBoundingBox());

		if (dist >= 0.f && dist < bestDist) {
			build = (int)i;
			bestDist = dist;
		}
	}
	return build;
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(465433);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 9.2: Building Example")) {
		app.Run(); 
	}
	
}
