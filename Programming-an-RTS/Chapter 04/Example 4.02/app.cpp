#pragma once
#include <common.h>
#include "intpoint.h"
#include "heightMap.h"
#include <glm/gtc/matrix_transform.hpp>

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::unique_ptr<Renderer::Font> _font;	
	float m_angle;	
	int _size;
	int _amplitude;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	m_angle = 0.f;		
	_size = 10;
	_amplitude = 5;
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

	_heightMap = std::make_unique<HEIGHTMAP>(_device.get(), INTPOINT(100, 100));
	bool res= _heightMap->CreateRandomHeightMap(rand() % 2000, _size / 10.f, _amplitude / 10.f, 9);
	ASSERT(res, "Unable to create random heightmap.");
	res = _heightMap->CreateParticles();
	ASSERT(res, "Unable to create heightmap particles.");
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	m_angle += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_SPACE)) {
		bool res = _heightMap->CreateRandomHeightMap(rand() % 2000, _size / 10.f, _amplitude / 10.f, 9);
		ASSERT(res, "Unable to create random heightmap.");
		res = _heightMap->CreateParticles();
		ASSERT(res, "Unable to create heightmap particles.");
	}
	
	if (IsKeyPressed(KEY_DOWN)&&_size > 1) {
		_size--;
		Sleep(100);
	}
	if (IsKeyPressed(KEY_UP) && _size < 20) {
		_size++;
		Sleep(100);
	}
	if (IsKeyPressed(KEY_LEFT) && _amplitude > 1) {
		_amplitude--;
		Sleep(100);
	}
	if (IsKeyPressed(KEY_RIGHT) && _amplitude < 15) {
		_amplitude++;
		Sleep(100);
	}
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	vec2 center = vec2(50.f);// mHeightmap->GetCenter();
	vec3 eye = vec3(center.x + cos(m_angle) * center.x * 2.0f, 15.f/*mHeightmap->m_maxHeight*/ * 8.0f, -center.y + sin(m_angle) * center.y * 2.0f);
	vec3 lookat = vec3(center.x, 0.f, -center.y);
	vec3 up(0.f, 1.f, 0.f);
	mat4 matWorld = mat4(1.f);
	mat4 matView = lookAtLH(eye, lookat, up);
	mat4 matProj;
	if (Core::GetAPI() == Core::API::Vulkan) {
		matProj = glm::perspectiveFovLH_ZO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
		matProj[1][1] *= -1;
	}
	else {
		matProj = glm::perspectiveFovLH_NO(glm::pi<float>() / 4, (float)_width, (float)_height, 1.f, 1000.f);
	}
	mat4 viewProj = matProj * matView;

	char buffer[512];
	sprintf_s(buffer, "Size: %d (UP/DOWN Arrow)", _size);
	_font->Draw(buffer, 110, 10);
	sprintf_s(buffer, "Persistance: %d (LEFT/RIGHT Arrow)", _amplitude);
	_font->Draw(buffer, 110, 30);
	_font->Draw("Redraw: SPACE", 110, 50);

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
	if (app.Init(800, 600, "Example 4.2: Heightmaps from Perlin Noise")) {
		app.Run();
	}
	return 0;
}
