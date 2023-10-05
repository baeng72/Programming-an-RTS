#pragma once
#include <common.h>
#include "intpoint.h"
#include "heightMap.h"
#include "terrain.h"
#include <glm/gtc/matrix_transform.hpp>

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::unique_ptr<Renderer::Font> _font;	
	Renderer::DirectionalLight _light;
	TERRAIN	_terrain;
	float _angle;	
	float _radius;
	bool _wireframe;
	int _image;
	int _numPatches;
	
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
	_image = 0;
	_wireframe = false;
	_numPatches = 4;
	
	srand(411678390);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	//_device->EnableGeometry(true);	
	_device->EnableLines(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	//_device->SetVSync(false);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);
	_shaderManager.reset(Renderer::ShaderManager::Create(_device.get()));
	_terrain.Init(_device.get(),_shaderManager, INTPOINT(100, 100));
	
	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.f, -1.f, 0.f));
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_angle += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;
		_terrain.SetWireframe(_wireframe);
		Sleep(100);
	}
	else if (IsKeyPressed(KEY_F)) {
		//Create terrain from file
		_image++;
		if (_image > 1)
			_image = 0;
		if (_terrain._heightMap) {
			_terrain._heightMap->_maxHeight = 10.f;
			if (_image == 0) {
				
				_terrain._heightMap->LoadFromFile( "../../../../Resources/Chapter 04/Example 4.04/textures/abe.jpg");
				_terrain.CreatePatches(_numPatches);
				
			}
			else if (_image == 1) {
				_terrain._heightMap->LoadFromFile("../../../../Resources/Chapter 04/Example 4.04/textures/smiley.bmp");
				_terrain.CreatePatches(_numPatches);

			}
		}
	}
	else if (IsKeyPressed(KEY_SPACE)) {
		//Generate random terrain
		_terrain.GenerateRandomTerrain(_numPatches);
		Sleep(300);
	}
	else if (IsKeyPressed(KEY_KP_ADD) && _radius < 200.f) {
		//zoom out
		_radius += deltaTime * 30.f;
	}
	else if (IsKeyPressed(KEY_KP_SUBTRACT)) {
		_radius -= deltaTime * 30.f;
	}
	else if (IsKeyPressed(KEY_UP) && _numPatches < 8) {
		//Increate the number of patches used
		_numPatches++;
		_terrain.CreatePatches(_numPatches);
		Sleep(300);
	}		
	else if (IsKeyPressed(KEY_DOWN) && _numPatches > 1) {
		//Decrease the number of patches used
		_numPatches--;
		_terrain.CreatePatches(_numPatches);
		Sleep(300);
	}

}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.

	glm::vec2 center = glm::vec2(50.f);
	glm::vec3 eye = glm::vec3(center.x + cos(_angle) * _radius,
			_radius,
		-center.y + sin(_angle) * _radius);
	glm::vec3 lookat = glm::vec3(center.x, 0.f, -center.y);
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::mat4 matModel = glm::mat4(1.f);
	glm::mat4 matView = glm::lookAtLH(eye, lookat, up);
	constexpr float fov = glm::radians(45.f);
	mat4 matProj;
	if (Core::GetAPI() == Core::API::Vulkan) {
		matProj = glm::perspectiveFovLH_ZO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
		matProj[1][1] *= -1;
	}
	else {
		matProj = glm::perspectiveFovLH_NO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
	}
	glm::mat4 viewProj = matProj * matView;

	_font->Draw("W: Toggle Wireframe", 10, 10, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("+/-: Zoom In/Out", 10, 30, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("SPACE: Randomize Terrain", 10, 50, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("UP/DOWN: Increate/Decrease Number of Patches", 10, 70, glm::vec4(0.f, 0.f, 0.f, 1.f));
	_font->Draw("F: Load HeighMap from File", 10, 90, glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	

	_device->StartRender();		

	_terrain.Render(viewProj,matModel,_light);
	_font->Render();

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

int main(int argc, char* argv[]) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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
	if (app.Init(800, 600, "Example 4.4: Creating the Terrain Mesh")) {
		app.Run();
	}
	return 0;
}
