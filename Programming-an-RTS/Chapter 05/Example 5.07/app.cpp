#pragma once
#include <common.h>
#include "mouse.h"
#include "object.h"
#include "camera.h"


class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	
	CAMERA _camera;
	MOUSE _mouse;
	std::vector<OBJECT> _mechs;
	
	Renderer::DirectionalLight _light;
	std::unique_ptr<Renderer::Line> _gridLine;
	
	std::unique_ptr<Renderer::Line2D> _line;
	bool _lodOn;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	_lodOn = true;
	srand(123456);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableGeometry(true);
	_device->EnableLines(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	//_device->SetClearColor(44.f / 255.f, 44.f / 255.f, 1.f, 1.f);

	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));


	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);


	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.0f, -1.f, 0.f));

	_camera.Init(GetWindowPtr());
	_camera._radius = 100.f;

	LoadObjectResources(_device.get(), _shadermanager);

	_mouse.Init(_device.get(), GetWindowPtr());

	_line.reset(Renderer::Line2D::Create(_device.get()));
	std::vector<Renderer::LineVertex> gridLineVerts;
	for (int i = -12; i <= 12; i++) {
		gridLineVerts.push_back({ vec3(-180.f,0.f,-i * 15.f),Color(0.f,0.f,0.f,1.f) });
		gridLineVerts.push_back({ vec3(180.f,0.f,-i * 15.f),Color(0.f,0.f,0.f,1.f) });
		gridLineVerts.push_back({ vec3(i * 15.f,0.f,-180.f),Color(0.f,0.f,0.f,1.f) });
		gridLineVerts.push_back({ vec3(i * 15.f,0.f,180.f),Color(0.f,0.f,0.f,1.f) });
	}
	_gridLine.reset(Renderer::Line::Create(_device.get(),gridLineVerts.data(),(uint32_t)gridLineVerts.size(),1.f,true));//this is a line list, not line strip

	//Add mechs
	for (int y = -12; y < 12; y++) {
		for (int x = -12; x < 12; x++) {
			if (rand() % 3 == 0) {
				float a = (rand() % (int)((glm::pi<float>() * 2.f) * 1000)) / 1000.f;
				_mechs.push_back(OBJECT(MECH, vec3(x * 15.f, 0.f, -y * 15.f), vec3(0.f, a, 0.f)));
			}
		}
	}
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	else if (IsKeyPressed(KEY_SPACE))
	{
		_lodOn = !_lodOn;
		Sleep(100);
	}
	_mouse.Update();
	_camera.Update(_mouse,deltaTime);
	
	_line->Update(_width, _height);
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		

	ViewPort v,v2;
	v.x = v.y = 0;
	v.width = 800;
	v.height = 600;
	v.ffar = 1.0f;
	v.fnear = 0.0f;
	v2 = v;
	//_device->SetViewport(v);
	
	glm::mat4 matView = _camera.GetViewMatrix();
	glm::mat4 matProj = _camera.GetProjectionMatrix();
	glm::mat4 viewProj = matProj * matView;
	_camera.CalculateFrustum(matProj, matView);	
	_gridLine->Draw(viewProj);

	//Draw Mechs
	long noFaces = 0;
	int noObjects = 0;

	for (int i = 0; i < _mechs.size(); i++) {
		if (_lodOn)
			_mechs[i].Render(&_camera, viewProj, _light, noFaces, noObjects);
		else
			_mechs[i].Render(nullptr, viewProj, _light, noFaces, noObjects);

	}
	
	//Top menu
	v.x = 0;
	v.y = 0;
	v.width = 800;
	v.height = 70;
	_device->SetViewport(v);
	Rect r = { 0,0,800,70 };
	_device->Clear(r, Color(1.f));
	_font->SetDimensions(800, 70);

	char buffer[64];
	sprintf_s(buffer, "Num Mechs in screen: %d/%d", noObjects, (int)_mechs.size());
	_font->Draw(buffer, 500, 10, Color(0.f, 0.f, 0.f, 1.f));

	//number of polygons
	sprintf_s(buffer, "Num Total Polygons: %d", noFaces);
	_font->Draw(buffer, 500, 30, Color(0.f, 0.f, 0.f, 1.f));

	//Controls
	_font->Draw("Mouse Wheel: Change Camera Radius", 10, 10, Color(0.f, 0.f, 0.f, 1.f));
	_font->Draw("Arrows: Change Camera Angle", 10, 30, Color(0.f, 0.f, 0.f, 1.f));

	if (_lodOn) {
		_font->Draw("Space: LOD & Culling = on", 10, 50, Color(0.f, 0.f, 0.f, 1.f));
	}else
		_font->Draw("Space: LOD & Culling = Off", 10, 50, Color(0.f, 0.f, 0.f, 1.f));
	
	_font->Render();
	_device->SetViewport(v2);
	
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
	if (app.Init(800, 600, "Example 5.7: Level-of-Detail Example")) {
		app.Run();
	}
	return 0;
}
