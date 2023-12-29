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
	
	
	_shader->SetUniform("viewProj", &matViewProj);
	_shader->SetUniform("model", &worldxform);
	_shader->SetUniform("light.ambient", &light.ambient);
	_shader->SetUniform("light.diffuse", &light.diffuse);
	_shader->SetUniform("light.specular", &light.specular);
	_shader->SetUniform("light.direction", &light.direction);
	_shader->Bind();
	for (int i = 0; i < _meshes.size(); i++) {
		
		_shader->SetUniform("color", &_meshColors[i]);
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
	
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("mesh.glsl"))));
	
}
