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
	Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
	int uboid = 0;


	Renderer::FlatShaderPushConst pushConst{matWorld*_xform};

	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	_mesh->Render(_shader.get());
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
	_texture = std::unique_ptr<Renderer::Texture>(model->GetTexture(Mesh::TextureType::diffuse, 0));
	_xform = model->GetMeshXForm(0);
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.09/shaders/mesh.glsl")));
	int texid = 0;
	std::vector<Renderer::Texture*> textures = { _texture.get() };
	_shader->SetTexture(texid, textures.data(), 1);
}
