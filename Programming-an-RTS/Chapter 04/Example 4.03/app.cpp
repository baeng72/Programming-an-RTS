#pragma once
#include <common.h>
#include "intpoint.h"
#include "heightMap.h"
#include <glm/gtc/matrix_transform.hpp>

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::unique_ptr<Renderer::Font> _font;	
	float _angle;	
	float _angle_b;	
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
	_angle_b = 0.5f;
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableGeometry(true);	
	_device->SetVSync(false);
	_device->Init();

	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);

	_heightMap = std::make_unique<HEIGHTMAP>(_device.get(), INTPOINT(50,50));
	
	bool res = _heightMap->CreateParticles();
	ASSERT(res, "Unable to create heightmap particles.");
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (IsKeyPressed(KEY_SPACE)) {
		_heightMap->SmoothTerrain();
	}
	//Move selection rectangle
	if (IsKeyPressed(KEY_A) && _heightMap->_selRect.left > 0)
		_heightMap->MoveRect(LEFT);
	if (IsKeyPressed(KEY_D) && _heightMap->_selRect.left < _heightMap->_size.x-1)
		_heightMap->MoveRect(RIGHT);
	if (IsKeyPressed(KEY_W) && _heightMap->_selRect.top > 0)
		_heightMap->MoveRect(UP);
	if (IsKeyPressed(KEY_S) && _heightMap->_selRect.top < _heightMap->_size.y)
		_heightMap->MoveRect(DOWN);
	//Raise/Lower heightmap
	if (IsKeyPressed(KEY_KP_ADD))
		_heightMap->RaiseTerrain(_heightMap->_selRect, deltaTime * 3.f);
	if (IsKeyPressed(KEY_KP_SUBTRACT))
		_heightMap->RaiseTerrain(_heightMap->_selRect, -deltaTime * 3.f);
	if (IsKeyPressed(KEY_UP) && _angle_b < glm::pi<float>() * 0.4f)
		_angle_b += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_DOWN) && _angle_b > 0.1f)
		_angle_b -= deltaTime * 0.5f;
	if (IsKeyPressed(KEY_LEFT))
		_angle -= deltaTime * 0.5f;
	if (IsKeyPressed(KEY_RIGHT))
		_angle += deltaTime * 0.5f;
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.

	vec2 center =_heightMap->GetCenter();
	vec3 eye = vec3(center.x + cos(_angle) * cos(_angle_b) * center.x * 1.5f,
			sin(_angle_b)*_heightMap->_maxHeight*5.f,
		-center.y + sin(_angle) * cos(_angle_b)*center.y*1.5f);
	vec3 lookat = vec3(center.x, 0.f, -center.y);
	vec3 up(0.f, 1.f, 0.f);
	mat4 matWorld = mat4(1.f);
	mat4 matView = glm::lookAtLH(eye, lookat, up);
	constexpr float fov = glm::radians(45.f);
	mat4 matProj;
	if (Core::GetAPI() == Core::API::Vulkan) {
		matProj = glm::perspectiveFovLH_ZO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
		matProj[1][1] *= -1;
	}
	else {
		matProj = glm::perspectiveFovLH_NO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
	}
	mat4 viewProj = matProj * matView;

	_font->Draw("Arrows: Move Camera", 10, 10);
	_font->Draw("A/W/S/D: Move Square", 10, 30);
	_font->Draw("+/-: Raise/Lower Square", 10, 50);
	_font->Draw("SPACE: Smooth Terrain", 10, 70);

	_device->StartRender();		

	_heightMap->Render(viewProj, eye);	
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
	if (app.Init(800, 600, "Example 4.3: Heightmap Editor")) {
		app.Run();
	}
	return 0;
}
