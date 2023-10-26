#pragma once
#include <common.h>
#include"skinnedMesh.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	Renderer::DirectionalLight _light;
	SKINNEDMESH _skinnedMesh;
	float _angle;
	float _unitTime;
	float _colFade;
	vec4 _activeCol;
	vec4 _lastCol;
	vec4 _currentCol;
	float _time;
	float _pressTime;
	int _col;
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
	_unitTime = _colFade = 0.f;
	_currentCol = _lastCol = _activeCol = vec4(1.f, 0.f, 0.f, 1.f);
	_time = _pressTime = 0.f;
	_col = 0;
}


bool APPLICATION::Init(int width, int height, const char* title){
	LOG_INFO("Application::Init()");
	
	if (!Application::Init(width, height, title))
		return false;

	

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);	
	_device->Init();	
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);

	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));

	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf",18);

	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.0f, -1.f, 0.f));	
	_skinnedMesh.Load(_device.get(), _shadermanager,"../../../../Resources/Chapter 08/Example 8.01/units/magician.x");
	
	
	_skinnedMesh.SetAnimation("Run");
	
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_time += deltaTime;
	_angle += deltaTime * 0.2f;
	_unitTime = deltaTime * 0.5f;
	if (_angle > glm::pi<float>() * 2.f)
		_angle -= glm::pi<float>() * 2.f;

	//change color
	_colFade += deltaTime * 0.4f;
	if (_colFade > 1.f)
		_colFade = 1.f;

	_currentCol = _lastCol * (1.f - _colFade) + _activeCol * _colFade;

	if (IsKeyPressed(KEY_SPACE)) {
		
		_pressTime = _time;
		_col++;
		if (_col > 11)
			_col = 0;
		vec4 colors[] = {
			vec4(1.0f, 0.0f, 0.0f, 1.0f),
								  vec4(0.0f, 1.0f, 0.0f, 1.0f),
								  vec4(0.0f, 0.0f, 1.0f, 1.0f),
								  vec4(1.0f, 1.0f, 0.0f, 1.0f),
								  vec4(1.0f, 0.0f, 1.0f, 1.0f),
								  vec4(0.0f, 1.0f, 1.0f, 1.0f),
								  vec4(0.5f, 0.25f, 0.0f, 1.0f),
								  vec4(0.0f, 0.0f, 0.0f, 1.0f),
								  vec4(1.0f, 0.5f, 0.0f, 1.0f),
								  vec4(0.0f, 0.25f, 0.0f, 1.0f),
								  vec4(0.25f, 0.0f, 0.0f, 1.0f),
								  vec4(0.0f, 0.0f, 0.25f, 1.0f)
		};
		_lastCol = _currentCol;
		_activeCol = colors[_col];
		_colFade = 0.f;
		Sleep(100);
	}
	_skinnedMesh.SetPose(deltaTime*0.5f);


}


void APPLICATION::Render() {
	_device->StartRender();
	Rect r = { 0,200,_width,400 };
	Color c = _currentCol;
	_device->Clear(r, c);
	mat4 matView = glm::lookAtLH(vec3(0.f, 10.f, -50.f), vec3(0.f, 4.f, 0.f), vec3(0.f, 1.f, 0.f));
	mat4 matProj;
	if (Core::GetAPI() == Core::API::Vulkan) {
		matProj = vulkOrthoLH(10.f, 9.f, 0.1f, 1000.f);
	}
	else {
		matProj = glOrthoLH(10.f, 9.f, 0.1f, 1000.f);
	}
	mat4 matVP = matProj * matView;

	//Set Skeleton to 		
	mat4 matWorld = glm::rotate(glm::scale(glm::mat4(1.f), vec3(1.3f)), _angle, vec3(0.f, 1.f, 0.f));		
	_skinnedMesh.Render(matVP, matWorld, _light,_currentCol);
		
	_font->Draw("Space: Change team color", 10, 10, Color(0.f, 0.f, 0.f, 1.f));
	
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
	if (app.Init(800, 600, "Example 8.1: Team Color")) {
		app.Run();
	}
	return 0;
}
