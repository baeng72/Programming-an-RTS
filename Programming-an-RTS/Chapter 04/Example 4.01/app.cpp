#pragma once
#include <common.h>
#include "intpoint.h"
#include "heightMap.h"
#include <glm/gtc/matrix_transform.hpp>

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::unique_ptr<HEIGHTMAP> _heightMap;
	int	_image;
	float m_angle;
	
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
	_image = 0;
	
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableGeometry(true);
	_device->EnableDepthBuffer(false);
	_device->SetVSync(true);
	_device->Init();

	_heightMap = std::make_unique<HEIGHTMAP>(_device.get(), INTPOINT(100, 100));
	_heightMap->LoadFromFile("../../../../Resources/Chapter 04/Example 4.01/textures/abe.jpg");
	
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	m_angle += deltaTime * 0.5f;
	if (IsKeyPressed(KEY_SPACE)) {
		_image++;
		_image %= 3;
		switch (_image) {
		case 0:
			_heightMap->LoadFromFile("../../../../Resources/Chapter 04/Example 4.01/textures/abe.jpg");
			break;
		case 1:
			_heightMap->LoadFromFile("../../../../Resources/Chapter 04/Example 4.01/textures/smiley.bmp");
			break;
		case 2:
			_heightMap->LoadFromFile("../../../../Resources/Chapter 04/Example 4.01/textures/heightmap.jpg");
			break;
			
		}
	}
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	glm::vec2 center = glm::vec2(50.f);// mHeightmap->GetCenter();
	glm::vec3 eye = glm::vec3(center.x + cos(m_angle) * center.x * 2.0f, 15.f/*mHeightmap->m_maxHeight*/ * 8.0f, -center.y + sin(m_angle) * center.y * 2.0f);
	glm::vec3 lookat = glm::vec3(center.x, 0.f, -center.y);
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::mat4 matWorld = glm::mat4(1.f);
	glm::mat4 matView = glm::lookAtLH(eye, lookat, up);
	glm::mat4 matProj = Core::perspective(quaterpi, (float)_width, (float)_height, 1.f, 1000.f);
	
	
	glm::mat4 viewProj = matProj * matView;

	_device->StartRender();		

	_heightMap->Render(viewProj, eye);	

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_device->Wait();
	_heightMap.reset();
}

void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(129111);
#endif
	
	APPLICATION app;
	if (app.Init(800, 600, "Example 4.1: Heightmaps from Images")) {
		app.Run();
	}
	
}
