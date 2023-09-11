#pragma once
#include <common.h>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

class APPLICATION : public Application {
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
	std::vector<glm::mat4> _xforms;
	std::vector<glm::vec4> _meshColors;
	std::vector<std::unique_ptr<Mesh::Mesh>> _meshes;
	std::unique_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Shader> _shader;
	Renderer::DirectionalLight _light;
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

	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(_device.get(), "../../../../Resources/Chapter 05/Example 5.01/meshes/terrain.x"));
	auto meshCount = model->GetMeshCount();
	_meshes.resize(meshCount);
	_meshColors.resize(meshCount);
	_xforms.resize(meshCount);
	for (uint32_t i = 0; i < meshCount; i++) {
		_meshes[i] = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::position_normal, i));
		auto matIdx = model->GetMeshMaterialIndex(i);
		auto mat = model->GetMaterial(i);
		_meshColors[i] = mat->diffuse;
		_xforms[i] = model->GetMeshXForm(i);
	}
	
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));

	_shader.reset(Renderer::Shader::Create(_device.get(), _shadermanager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.01/shaders/mesh.glsl")));

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
	auto viewProj = matProj * matView;
	Uniform u{ viewProj,_light };
	uint32_t uid = 0;
	_shader->SetUniformData(uid, &u, sizeof(u));
}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		
	glm::mat4 model = glm::mat4(1.f);
	for (size_t i = 0; i < _meshes.size(); i++) {
		PushConst pushConst = { _xforms[i],_meshColors[i] };
		_shader->SetPushConstData(&pushConst, sizeof(pushConst));
		_meshes[i]->Render(_shader.get());
	}

	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
	
}

void APPLICATION::Cleanup() {
	_device->Wait();
	_meshes.clear();
}

int main() {
	APPLICATION app;
	if (app.Init(800, 600, "Example 5.1: Camera Example")) {
		app.Run();
	}
	return 0;
}
