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
	_shader->Bind();
	mat4 worldxform = matWorld * _xform;
	if (Core::GetAPI() == Core::API::Vulkan) {
		Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
		int uboid = 0;


		Renderer::FlatShaderPushConst pushConst{ worldxform };

		_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
		_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	}
	else {
		_shader->SetUniformData("viewProj", &matViewProj, sizeof(mat4));
		_shader->SetUniformData("model", &worldxform, sizeof(mat4));
		_shader->SetUniformData("light.ambient", &light.ambient, sizeof(vec4));
		_shader->SetUniformData("light.diffuse", &light.diffuse, sizeof(vec4));
		_shader->SetUniformData("light.specular", &light.specular, sizeof(vec4));
		_shader->SetUniformData("light.direction", &light.direction, sizeof(vec3));
		auto texture = _texture.get();
		_shader->SetTexture("texmap", &texture, 1);
	}
	
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
	if (Core::GetAPI() == Core::API::Vulkan) {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.09/shaders/Vulkan/mesh.glsl")));
		int texid = 0;
		std::vector<Renderer::Texture*> textures = { _texture.get() };
		_shader->SetTexture(texid, textures.data(), 1);
	}
	else {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.09/shaders/GL/mesh.glsl")));
	}
}
