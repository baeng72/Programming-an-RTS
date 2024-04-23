#pragma once
#define __USE__PROFILER__
#include <common.h>
#include "intpoint.h"
#include "mouse.h"
#include "camera.h"
#include "terrain.h"
#include "building.h"
#include "player.h"
#include "effect.h"
#include "particles.h"

extern std::vector<EFFECT*> effects;



class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::shared_ptr<Renderer::Shader> _buildingShader;
	std::unique_ptr<Renderer::Font> _font;		
	std::unique_ptr<Renderer::Line2D> _line;
	Renderer::DirectionalLight _light;	
	TERRAIN _terrain;
	CAMERA _camera;
	MOUSE _mouse;
	std::vector<PLAYER*> _players;
	int _thisPlayer;
	float _gameSpeed;
	bool _wireframe;
	bool _paused;
	
	void AddPlayers(int noPlayers);
	
	
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
	
	srand(107696968);
	
	Core::ResourcePath::SetProjectPath("Chapter 14/Example 14.01");
	_gameSpeed = 3.f;
	_paused = false;
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init() thread id: {0}",std::this_thread::get_id());
	if (!Application::Init(width, height, title))
		return false;

	_threads = std::make_unique<Core::ThreadPool>(8);
	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_time = _device->GetCurrentTicks();
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	
	_buildingShader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData(Core::ResourcePath::GetShaderPath("building.glsl"))));
	
	_line.reset(Renderer::Line2D::Create(_device.get()));
	_line->Update(_width, _height);
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(vec3(.5f, 1.f, -0.5f));
	LoadObjectResources(_device.get());
	LoadMapObjectResources(_device.get());
	LoadBuildingResources(_device.get());
	LoadUnitResources(_device.get(),_shadermanager);
	LoadPlayerResources(_device.get(), _shadermanager);
	LoadEffectResources(_device.get(), _shadermanager);
	LoadParticleResources(_device.get(), _shadermanager);

	_mouse.Init(_device.get(), _shadermanager, GetWindowPtr());

	_camera.Init(GetWindowPtr());
	//_camera._focus = vec3(50, 10, -50);
	_camera._fov = 0.6f;
	_camera._radius = 50.f;

	_terrain.Init(_device.get(),GetWindowPtr(), _shadermanager, INTPOINT(150, 150));

	

	

	AddPlayers(4);
	_thisPlayer = 0;
	return true;
}

void APPLICATION::Update(float deltaTime) {

	deltaTime *= 10;
	//remove dead effects
	std::vector<EFFECT*> toDelete;
	for (int i = 0; i < effects.size(); i++) {
		if (effects[i]->isDead())
			toDelete.push_back(effects[i]);
		else
			effects[i]->Update(deltaTime * _gameSpeed);
	}

	if (_paused && _gameSpeed > 0.f) {
		_gameSpeed -= deltaTime * 2.f;
		_gameSpeed = std::max(_gameSpeed, 0.f);		

	}
	else if (!_paused && _gameSpeed < 3.f) {
		_gameSpeed += deltaTime * 2.f;
		_gameSpeed = std::min(_gameSpeed, 3.f);
	}
	
	
	//update players
	//for (auto& player : _players) {
	for(size_t i=0;i<_players.size();i++){
		auto player = _players[i];
		if (player) {
			//player->UpdateMapObjects(deltaTime);
			_threads->QueueJob([player, deltaTime]() {player->UpdateMapObjects(deltaTime); });
		}
	}
	_terrain.RenderFogOfWar(_players[_thisPlayer]);

	_camera.Update(_mouse, _terrain, deltaTime);
	_mouse.Update(_terrain);

	if (_terrain._updateSight) {
		_terrain._updateSight = false;
		if (_thisPlayer < _players.size() && _players[_thisPlayer])
			_terrain.UpdateSightMatrixes(_players[_thisPlayer]->_mapObjects);

		for (int i = 0; i < _players.size(); i++) {
			if (_players[i]) {
				_players[i]->IsMapObjectsVisible();
			}
		}
		
		_terrain.UpdateMinimap(_players);
	}


	

	
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;		
		
		_terrain.SetWireframe(_wireframe);
		
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		//Generate random terrain		
		if (_gameSpeed == 0.f || _gameSpeed == 3.f)
			_paused = !_paused;
		
	}
	
	else if (IsKeyPressed(KEY_F)) {
		_camera._focus = vec3(75, 10, -75);//recenter if mouse goes wonkey, need to fix mouse
	}
	
}




void APPLICATION::Render() {
	EASY_FUNCTION();
	//using DirectX LHS coordinate system.
	vec4 viewport{ 0,0,(float)_width,(float)_height };
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = glm::mat4(1.f);
	glm::mat4 matProj = glm::mat4(1.f);
	glm::mat4 viewProj = glm::mat4(1.f);
	
	
	_device->SetClearColor(0.f, 0.f, 0.f, 1.f);
	matWorld = mat4(1.f);
	{
		EASY_BLOCK("Get Camera");
		matView = _camera.GetViewMatrix();
		matProj = _camera.GetProjectionMatrix();
	}
	{
		EASY_BLOCK("Frustum");
		_camera.CalculateFrustum(matProj, matView);
	}
	{
		EASY_BLOCK("Mouse Mappos")
			_mouse.CalculateMappos(matProj, matView, _terrain);

	}
	viewProj = matProj * matView;
	{
		EASY_BLOCK("Start Render");
		_device->StartRender();
	}
	//_terrain.RenderLandscape();
	//_terrain.UpdateMinimap(_players);
	{
		EASY_BLOCK("Render Terrain");
		_terrain.Render(viewProj, matWorld, _light, _camera);
	}
	{
		EASY_BLOCK("Render Players");
		for(size_t i=0;i<_players.size();i++){
			auto player = _players[i];
		//for (auto& player : _players) {
			if (player) {

				_players[i]->RenderMapObjects(_camera, _light);
			}
		}

		for (auto& effect : effects) {
			if (effect)
				effect->Render(matProj, matView);
		}

	}
	{
		EASY_BLOCK("Paint Selected");
		if (_players.size() && _players[0]) {
			_players[0]->PaintSelectedMapObjects(_camera);
			_players[0]->Select(matProj, matView, _mouse);
		}
	}
	{
		EASY_BLOCK("Render Minimap");
		Rect r = { 611, 9, 791, 189 };
		_line->Update(180, 180);
		_terrain.RenderMinimap(_camera,_mouse, r,_line.get());
		_line->Update(_width, _height);
	}
	{
		EASY_BLOCK("Font");
		
		_font->Draw("SPACE: Pause Game", 10, 580, glm::vec4(1.f, 1.f, 1.f, 1.f));
		_font->Draw("Kills:", 10, 10);
		const char* teamNames[] = { "RED", "GREEN", "BLUE","YELLOW" };
		char buffer[64];
		for (int i = 0; i < 4; i++) {
			sprintf(buffer, "%s: %d", teamNames[i], _players[i]->_numKills);
			_font->Draw(buffer,10, 35 + i * 20);
		}
		_font->Render();
	}
	{
		EASY_BLOCK("Paint Mouse");
		_mouse.Paint(viewProj, _light);
	}
	{
		EASY_BLOCK("EndRender");
		_device->EndRender();
	}
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop();
	_device->Wait();

	for (auto player : _players)
		delete player;
	UnloadPlayerResources();
	UnloadUnitResources();
	UnloadBuildingResources();
	UnloadMapObjectResources();
	UnloadObjectResources();
	UnloadEffectResources();
	UnloadParticleResources();
	_font.reset();
	_line.reset();
	_terrain.Cleanup();
}

void APPLICATION::AddPlayers(int noPlayers) {
	_thisPlayer = 0;
	for (auto& player : _players) {
		if (player)
			delete player;
	}
	_players.clear();

	INTPOINT startLocations[] = { INTPOINT(30,30), INTPOINT(120,30), INTPOINT(30,120), INTPOINT(120,120) };
	vec4 teamCols[] = { vec4(1.f,0.f,0.f,1.f),vec4(0.f,1.f,0.f,1.f),vec4(0.f,0.f,1.f,1.f),vec4(1.f,1.f,0.f,1.f) };
	int controllers[] = { COMPUTER, COMPUTER, COMPUTER, COMPUTER };
	if (noPlayers < 2)
		noPlayers = 2;
	if (noPlayers > 4)
		noPlayers = 4;

	for (int i = 0; i < noPlayers; i++) {
		_terrain.Progress("Creating Players", i / (float)noPlayers);
		_players.push_back(new PLAYER(i,controllers[i],teamCols[i], startLocations[i], &_terrain, _device.get()));
	}
	//Center camera focus on the team...
	_camera._focus = _terrain.GetWorldPos(_players[_thisPlayer]->GetCenter());
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(715330);
#endif
#if defined __USE__PROFILER__
	EASY_PROFILER_ENABLE;
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 14.1: Game Brains!")) {
		app.Run();
	}
#if defined __USE__PROFILER__
	char buffer[256];
	sprintf_s(buffer, "Example 14.1: Game Brains %s.prof", Core::GetAPI() == Core::API::GL ? "GL" : "Vulkan");
	profiler::dumpBlocksToFile(buffer);
#endif
	
}
