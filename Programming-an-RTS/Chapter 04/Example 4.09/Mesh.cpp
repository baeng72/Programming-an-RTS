#pragma once
#include "Mesh.h"



MESH::MESH() :_pdevice(nullptr) {

}

MESH::MESH( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shadermanager, const char* pfilename):_pdevice(pdevice),_shaderManager(shadermanager) {
	Load( pdevice,shadermanager, pfilename);
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
	auto texture = _texture.get();
	_shader->SetTexture("texmap", &texture, 1);
	_shader->Bind();
	_mesh->Bind();
	_mesh->Render();
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	_mesh.reset();
}

bool MESH::Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shaderManager, const char* pfilename) {
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));
	_mesh = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::position_normal_uv, 0));
	_texture = std::unique_ptr<Renderer::Texture>(model->GetTexture(model->GetMeshMaterialIndex(0), Mesh::TextureType::diffuse, 0));
	_xform = model->GetMeshXForm(0);
	LoadShader();
	return true;
}





void MESH::LoadShader()
{	
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("mesh.glsl"))));	
}
