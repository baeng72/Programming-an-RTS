#pragma once
#include <common.h>
#include "mouse.h"
#include "object.h"
#include "camera.h"
#include "City.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;	
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	
	CAMERA _camera;
	MOUSE _mouse;
	CITY _city;
	
	Renderer::DirectionalLight _light;
	std::unique_ptr<Renderer::Line2D> _line;
	
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	
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
	_device->Init();
	//_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_device->SetClearColor(44.f / 255.f, 44.f / 255.f, 1.f, 1.f);

	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));


	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);


	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.0f, -1.f, 0.f));

	_camera.Init(GetWindowPtr());

	LoadObjectResources(_device.get(), _shadermanager);

	_mouse.Init(_device.get(), GetWindowPtr());

	_line.reset(Renderer::Line2D::Create(_device.get()));

	_city.Init(INTPOINT(25, 25));


	_camera._focus = _city.GetCenter();
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_mouse.Update();
	_camera.Update(_mouse,deltaTime);
	
	_line->Update(_width, _height);
}

//glm is different for whatever reason
inline glm::mat4 D3DXOrthoLH(float width, float height, float zn, float zf) {
	glm::mat4 mat = glm::mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / height;
	mat[2][2] = 1.f / (zf - zn);
	mat[3][2] = -zn / (zf - zn);
	mat[1][1] *= -1;//flip y for Vulkan
	return mat;
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
	_city.Render(&_camera, viewProj, _light);

	

	//Render city overview, set viewport
	v.x = 550;
	v.y = 20;
	v.width = 230;
	v.height = 230;
	v.ffar = 1.0f;
	v.fnear = 0.0f;
	_device->SetViewport(v);
	Rect r = { (int)v.x,(int)v.y,(int)(v.x+v.width),(int)(v.y+v.height) };
	_device->Clear(r, Color(1.f));
	////Setup camera view to orthogonal view looking down on city
	mat4 viewTop = glm::lookAtLH((_city.GetCenter() + vec3(0.f, 100.f, 0.f)), _city.GetCenter(), vec3(0.f, 0.f, 1.f));
	mat4 projectionTop;
	if (Core::GetAPI() == Core::API::Vulkan) {
		projectionTop = vulkOrthoLH((float)_city._size.x * TILE_SIZE, (float)_city._size.y * TILE_SIZE, 0.01f, 1000.f);
	}
	else {
		projectionTop = glOrthoLH((float)_city._size.x * TILE_SIZE, (float)_city._size.y * TILE_SIZE, 0.01f, 1000.f);
	}
	//mat4 projectionTop = D3DXOrthoLH((float)_city._size.x * TILE_SIZE,  (float)_city._size.y * TILE_SIZE, 0.01f, 1000.f);
	
	mat4 viewProjTop = projectionTop * viewTop;
	_city.Render(nullptr, viewProjTop, _light);

	//Restore viewport
	_device->SetViewport(v2);

	////Draw line around smaller window
	vec2 outline[] = { vec2(550,20),vec2(779,20),vec2(779,249),vec2(550,249),vec2(550,20) };
	_line->Draw(outline, 5,vec4(0.f), 3.f);
	
	_font->Draw("Mouse Wheel: Change Camera Radius", 10, 10, Color(0.f,0.f,0.f,1.f));
	_font->Draw("Arrows: Change Camera Angle", 10, 30, Color(0.f, 0.f, 0.f, 1.f));
	_font->Render();
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
	if (app.Init(800, 600, "Example 5.6: Frustum Culling")) {
		app.Run();
	}
	return 0;
}
