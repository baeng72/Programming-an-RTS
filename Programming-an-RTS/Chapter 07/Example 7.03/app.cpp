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
	std::vector<std::string> _animations;
	int _activeAnimation;
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
	_skinnedMesh.Load(_device.get(), _shadermanager,"../../../../Resources/Chapter 07/Example 7.03/mesh/drone.x");
	_animations = _skinnedMesh.GetAnimations();
	_activeAnimation = (int)_animations.size() - 3;
	_skinnedMesh.SetAnimation(_animations[_activeAnimation].c_str());
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_angle += deltaTime * 0.5f;
	if (_angle > glm::pi<float>() * 2.f)
		_angle -= glm::pi<float>() * 2.f;

	if (IsKeyPressed(KEY_SPACE)) {
		_activeAnimation++;
		if (_activeAnimation >= _animations.size()) {
			_activeAnimation = 0;
		}
		_skinnedMesh.SetAnimation(_animations[_activeAnimation].c_str());
	}
	_skinnedMesh.SetPose(deltaTime*0.5f);


}
////glm is different for whatever reason
//inline glm::mat4 D3DXOrthoLH(float width, float height, float zn, float zf) {
//	glm::mat4 mat = glm::mat4(1.f);
//	mat[0][0] = 2.f / width;
//	mat[1][1] = 2.f / height;
//	mat[2][2] = 1.f / (zf - zn);
//	mat[3][2] = -zn / (zf - zn);
//	mat[1][1] *= -1;//flip y for Vulkan
//	return mat;
//}

void APPLICATION::Render() {
	_device->StartRender();
	mat4 matView = glm::lookAtLH(vec3(0.f, 10.f, -50.f), vec3(0.f, 4.f, 0.f), vec3(0.f, 1.f, 0.f));
	mat4 matProj = Core::orthoWH(10.f, 9.f, 0.1f, 1000.f);
	//mat4 matProj = D3DXOrthoLH(10.f, 9.f, 0.1f, 1000.f);
	mat4 matVP = matProj * matView;
	//Set Skeelton to 
	
	mat4 matWorld = glm::rotate(glm::scale(glm::mat4(1.f), vec3(1.3f)), _angle, vec3(0.f, 1.f, 0.f));
	_skinnedMesh.Render(matVP, matWorld, _light);
	
	_font->Draw("Space: Change Animation", 10, 10, Color(0.f, 0.f, 0.f, 1.f));
	char buffer[128];
	sprintf_s(buffer, "Active Animation: %s", _animations[_activeAnimation].c_str());
	_font->Draw(buffer, 10, 50, Color(0.f, 0.f, 0.f, 1.f));
	_font->Render();
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(133923);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 7.3: Skinned Mesh Example")) {
		app.Run();
	}
	
}
