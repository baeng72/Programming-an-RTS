#pragma once
#include <common.h>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

class APPLICATION : public Core::Application {
	struct PushConst {
		glm::mat4 model;
		glm::vec4 color;
	};
	struct Uniform {
		glm::mat4 viewProj;
		Renderer::DirectionalLight light;
	};
	std::unique_ptr<Renderer::RenderDevice> _device;		
	CAMERA _camera;
	/*std::vector<glm::mat4> _xforms;
	std::vector<glm::vec4> _meshColors;
	std::vector<std::unique_ptr<Mesh::Mesh>> _meshes;*/
	mat4 _xform;
	std::unique_ptr<Mesh::MultiMesh> _multiMesh;
	std::vector<vec4> _meshColors;
	std::unique_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Shader> _shader;
	Renderer::DirectionalLight _light;
	mat4 _matVP;
	
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;
	
	
	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);
	const char* path = "../../../../Resources/Chapter 05/Example 5.01/meshes/terrain.x";
	
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(_device.get(), path));
	std::unique_ptr<Mesh::MultiMesh> multiMesh = std::unique_ptr<Mesh::MultiMesh>(model->GetMultiMesh(Mesh::MeshType::position_normal));
	uint32_t partCount = multiMesh->GetPartCount();

	std::vector<vec4> diffuseMats(partCount);
	for (uint32_t i = 0; i < partCount; i++) {
		auto matid = multiMesh->GetMaterialIndex(i);
		auto mat = model->GetMaterial(matid);
		diffuseMats[i] = mat->diffuse;
	}
	multiMesh->GetWorldXForm(_xform);
	_meshColors = diffuseMats;
	_multiMesh = std::move(multiMesh);
	
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	if (Core::GetAPI() == Core::API::Vulkan) {
		_shader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.01/shaders/Vulkan/mesh.glsl")));
	}
	else {
		_shader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.01/shaders/GL/mesh.glsl")));
	}

	_light.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);
	_light.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	_light.direction = glm::normalize(glm::vec3(0.7f, -0.3f, 0.f));

	_camera.Init(GetWindowPtr());
	
	return true;
}



void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_camera.Update(deltaTime);
	auto matView = _camera.GetViewMatrix();
	auto matProj = _camera.GetProjectionMatrix();
	_matVP = matProj * matView;
	
	
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		
	glm::mat4 model = glm::mat4(1.f);
	_shader->Bind();
	_multiMesh->Bind();
	if (Core::GetAPI() == Core::API::Vulkan) {
		uint32_t uid = 0;
		Uniform u{ _matVP,_light };
		_shader->SetUniformData(uid, &u, sizeof(u));
	}
	else {
		_shader->SetUniformData("viewProj", &_matVP, sizeof(mat4));
		_shader->SetUniformData("light.ambient", &_light.ambient, sizeof(vec4));
		_shader->SetUniformData("light.diffuse", &_light.diffuse, sizeof(vec4));
		_shader->SetUniformData("light.specular", &_light.specular, sizeof(vec4));
		_shader->SetUniformData("light.direction", &_light.direction, sizeof(vec3));
	}
	for (uint32_t i = 0; i < _multiMesh->GetPartCount(); i++) {
		if (Core::GetAPI() == Core::API::Vulkan) {
			PushConst pushConst = { _xform,_meshColors[i] };
			_shader->SetPushConstData(&pushConst, sizeof(pushConst));
		}
		else {
			_shader->SetUniformData("model", &_xform, sizeof(mat4));
			_shader->SetUniformData("color", &_meshColors[i], sizeof(vec4));
		}
		_multiMesh->Render(i);
		
		
	}
	/*for (size_t i = 0; i < _meshes.size(); i++) {
		PushConst pushConst = { _xforms[i],_meshColors[i] };
		_shader->SetPushConstData(&pushConst, sizeof(pushConst));
		_meshes[i]->Bind();
		_meshes[i]->Render();
	}*/

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
	
}

void APPLICATION::Cleanup() {
	
	_device->Wait();
	//_meshes.clear();
	_multiMesh.reset();
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
	if (app.Init(800, 600, "Example 5.1: Camera Example")) {
		app.Run();
	}
	return 0;
}
