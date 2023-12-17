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
	_shader->Bind();
	//if (Core::GetAPI() == Core::API::Vulkan) {
	//	Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
	//	int uboid = 0;


	//	Renderer::FlatShaderPushConst pushConst{ worldxform };

	//	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	//	//_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	//	_shader->SetUniformData("PushConst", &pushConst, sizeof(pushConst));
	//}
	//else {
	//	_shader->SetUniformData("viewProj", &matViewProj, sizeof(mat4));
	//	_shader->SetUniformData("model", &worldxform, sizeof(mat4));
	//	_shader->SetUniformData("light.ambient", &light.ambient, sizeof(vec4));
	//	_shader->SetUniformData("light.diffuse", &light.diffuse, sizeof(vec4));
	//	_shader->SetUniformData("light.specular", &light.specular, sizeof(vec4));
	//	_shader->SetUniformData("light.direction", &light.direction, sizeof(vec3));
	//	auto texture = _texture.get();
	//	_shader->SetTexture("texmap", &texture, 1);
	//}
	_shader->SetUniform("viewProj", &matViewProj);
	_shader->SetUniform("model", &worldxform);
	_shader->SetUniform("light.ambient", &light.ambient);
	_shader->SetUniform("light.diffuse", &light.diffuse);
	_shader->SetUniform("light.specular", &light.specular);
	_shader->SetUniform("light.direction", &light.direction);
	auto texture = _texture.get();
	_shader->SetTexture("texmap", &texture, 1);
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
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.14/shaders/Vulkan/mesh.glsl")));
		
	}
	else {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.14/shaders/GL/mesh.glsl")));
	}
}

MESHINSTANCE::MESHINSTANCE()
{
	_pos = _rot = glm::vec3(0.f);
	_sca = glm::vec3(1.f);
}

MESHINSTANCE::MESHINSTANCE(MESH* meshPtr)
{
	_mesh = meshPtr;
	_pos = _rot = glm::vec3(0.f);
	_sca = glm::vec3(1.f);
}

void MESHINSTANCE::Render(glm::mat4&viewProj,Renderer::DirectionalLight&light)
{
	if (_mesh) {
		glm::mat4 p, rx,ry,rz,r, s;
		glm::mat4 identity = glm::mat4(1.f);
		p = glm::translate(identity, _pos);
		rx = glm::rotate(identity, _rot.x, glm::vec3(1.f, 0.f, 0.f));
		ry = glm::rotate(identity, _rot.y, glm::vec3(0.f, 1.f, 0.f));
		rz = glm::rotate(identity, _rot.z, glm::vec3(0.f, 0.f, 1.f));

		r = rz * ry * rx;
		s = glm::scale(identity, _sca);
		glm::mat4 world = p * r * s;
		_mesh->Render(viewProj, world, light);
	}
}
