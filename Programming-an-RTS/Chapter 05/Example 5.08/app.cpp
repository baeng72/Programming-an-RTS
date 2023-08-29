#pragma once
#include <common.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"


class APPLICATION : public Application {
	struct PushConst {
		glm::mat4 model;
	};
	struct Uniform {
		glm::mat4 viewProj;
		Renderer::DirectionalLight light;
	};
	
	std::unique_ptr<Renderer::RenderDevice> _device;		
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	Renderer::DirectionalLight _light;

	mat4 _xform;
	std::vector<glm::vec4> _meshColors;
	std::unique_ptr<MESH> _farmerMesh;
	MESHINSTANCE _farmer1;
	MESHINSTANCE _farmer2;
	bool _wireframe;
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
	srand(12345);
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	

	
	_farmerMesh = std::make_unique<MESH>();
	_farmerMesh->Load(_device.get(), _shadermanager, "../../../../Resources/Chapter 05/Example 5.08/objects/farmer.x");
	_farmer1.SetMesh(_farmerMesh.get());
	_farmer2.SetMesh(_farmerMesh.get());
	_farmer1.SetPosition(vec3(-2.5f, 0.f, 0.f));
	_farmer2.SetPosition(vec3(2.5f, 0.f, 0.f));
	
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);

	

	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.0f, -1.f, 0.f));

	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	//Rotate farmers
	_farmer1.SetRotation(_farmer1._rot + vec3(0.0f, deltaTime * 0.5f, 0.0f));
	if (_farmer1._rot.y > glm::pi<float>() * 2.0f)
		_farmer1._rot.y -= glm::pi<float>() * 2.0f;

	_farmer2.SetRotation(_farmer2._rot + vec3(0.0f, deltaTime * 0.5f, 0.0f));
	if (_farmer2._rot.y > glm::pi<float>() * 2.0f)
		_farmer2._rot.y -= glm::pi<float>() * 2.0f;

	if (IsKeyPressed(KEY_DOWN)) {
		_farmer2.SetLOD(_farmer2.GetNumProgressiveFaces() - 5);
		Sleep(30);
	}
	else if (IsKeyPressed(KEY_UP)) {
		_farmer2.SetLOD(_farmer2.GetNumProgressiveFaces() + 5);
		Sleep(30);
	}
	else if (IsKeyPressed(KEY_W)) {
		_wireframe = !_wireframe;
		_farmerMesh->SetWireframe(_wireframe);
		Sleep(300);
	}

	
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

	mat4 matView = glm::lookAtLH(vec3(0.f, 10.f, -50.f), vec3(0.f, 3.f, 0.f), vec3(0.f, 1.f, 0.f));
	//mat4 matProj = glm::ortho(0.f, 10.f, 0.f, 9.f, 0.1f, 1000.f);
	//matProj[1][1] *= -1;
	mat4 matProj = D3DXOrthoLH(10.f, 9.f, 0.1f, 1000.f);
	
	mat4 matVP = matProj * matView;
	_farmer1.Render(matVP, _light);
	_farmer2.RenderProgressive(matVP, _light);

	//Number of polygons
	char buffer[64];
	_font->Draw("Original", 170, 520, Color(0.f, 0.f, 0.f, 1.f));
	_font->Draw("Progresive Mesh", 530, 520, Color(0.f, 0.f, 0.f, 1.f));
	sprintf_s(buffer, "%d polygons (UP/DOWN Arrow)", _farmer2.GetNumProgressiveFaces());
	_font->Draw(buffer, 470, 540, Color(0.f, 0.f, 0.f, 1.f));
	_font->Draw("(W)ireframe On/Off", 130, 540, Color(0.f, 0.f, 0.f, 1.f));
	_font->Render();
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
	
}

void APPLICATION::Cleanup() {
	_device->Wait();
	_farmerMesh.reset();
}

int main() {
	APPLICATION app;
	if (app.Init(800, 600, "Example 5.8: Progressive Meshes")) {
		app.Run();
	}
	return 0;
}
