#pragma once
#include "Mesh.h"



MESH::MESH() :_pdevice(nullptr) {

}

MESH::MESH(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shadermanager, const char* pfilename) : _pdevice(pdevice), _shaderManager(shadermanager) {
	Load(pdevice, shadermanager, pfilename);
}

MESH::~MESH() {
	Release();
}
void MESH::Render(glm::mat4& matViewProj, glm::mat4& matWorld, Renderer::DirectionalLight& light)
{
	
	mat4 worldxform = matWorld * _xform;
	_shader->Bind();
	struct PushConst {
		mat4 world;
		vec4 color;
	}pushConst{ worldxform,vec4(1.f) };
	if (Core::GetAPI() == Core::API::Vulkan) {
		Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
		int uboid = 0;


		

		_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
		
		
	}
	else {
		_shader->SetUniformData("viewProj", &matViewProj, sizeof(mat4));
		_shader->SetUniformData("model", &worldxform, sizeof(mat4));
		_shader->SetUniformData("light.ambient", &light.ambient, sizeof(vec4));
		_shader->SetUniformData("light.diffuse", &light.diffuse, sizeof(vec4));
		_shader->SetUniformData("light.specular", &light.specular, sizeof(vec4));
		_shader->SetUniformData("light.direction", &light.direction, sizeof(vec3));
		
	}
	for (int i = 0; i < _meshes.size(); i++) {
		if (Core::GetAPI() == Core::API::Vulkan) {
			pushConst.color = _meshColors[i];
			_shader->SetPushConstData(&pushConst, sizeof(pushConst));
			
		}
		else {
			_shader->SetUniformData("color", &_meshColors[i], sizeof(vec4));
			
		}
		_meshes[i]->Bind();
		_meshes[i]->Render();
	}
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	_shader.reset();
	for (auto& mesh : _meshes) {
		mesh.reset();
	}
}

bool MESH::Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shaderManager, const char* pfilename) {
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));
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
	_xform = _xforms[0];
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	if (Core::GetAPI() == Core::API::Vulkan) {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.04/shaders/Vulkan/mesh.glsl")));
	}
	else {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.04/shaders/GL/mesh.glsl")));
	}
	
}
