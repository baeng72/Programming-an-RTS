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
	Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
	int uboid = 0;
	struct PushConst {
		glm::mat4 model;
		glm::vec4 color;
	}pushConst;

	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	glm::mat4 model = glm::mat4(1.f);
	for (size_t i = 0; i < _meshes.size(); i++) {
		pushConst = { matWorld * _xforms[i],_meshColors[i] };
		_shader->SetPushConstData(&pushConst, sizeof(pushConst));
		_meshes[i]->Render(_shader.get());
	}
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
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
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.04/shaders/mesh.glsl")));
	
}
