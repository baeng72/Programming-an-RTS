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
	_multiMesh->Bind();
	_shader->SetUniform("light.ambient", &light.ambient);
	_shader->SetUniform("light.diffuse", &light.diffuse);
	_shader->SetUniform("light.specular", &light.specular);
	_shader->SetUniform("light.direction", &light.direction);
	for (uint32_t i = 0; i < _multiMesh->GetPartCount(); i++) {		
		_shader->SetUniform("viewProj", &matViewProj);
		_shader->SetUniform("model", &worldxform);		
		_shader->SetUniform("clrdiffuse", &_meshColors[i]);
		_shader->SetUniform("clrspecular", &_meshSpeculars[i]);
		_multiMesh->Render(i);
	}

	
	
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	_multiMesh.reset();
}

bool MESH::Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shaderManager, const char* pfilename) {
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));

	std::unique_ptr<Mesh::MultiMesh> multiMesh = std::unique_ptr<Mesh::MultiMesh>(model->GetMultiMesh(Mesh::MeshType::position_normal));
	uint32_t partCount = multiMesh->GetPartCount();

	std::vector<vec4> diffuseMats(partCount);
	std::vector<vec4> specularMats(partCount);
	for (uint32_t i = 0; i < partCount; i++) {
		auto matid = multiMesh->GetMaterialIndex(i);
		auto mat = model->GetMaterial(matid);
		diffuseMats[i] = mat->diffuse;
		specularMats[i] = mat->specular;
	}
	multiMesh->GetWorldXForm(_xform);
	_meshColors = diffuseMats;
	_meshSpeculars = specularMats;
	_multiMesh = std::move(multiMesh);
	
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	if (Core::GetAPI() == Core::API::Vulkan) {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.05/shaders/Vulkan/mesh.glsl")));
	}
	else {
		_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.05/shaders/GL/mesh.glsl")));
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
		
		
		glm::mat4 world = GetWorldMatrix();
		_mesh->Render(viewProj, world, light);
	}
}

mat4 MESHINSTANCE::GetWorldMatrix()
{
	mat4 p, rx,ry,rz,r, s;
	mat4 id = glm::mat4(1.f);

	p = glm::translate(id, _pos);
	//r = glm::rotate(glm::rotate(glm::rotate(id, _rot.x, vec3(1.f, 0.f, 0.f)), _rot.y, vec3(0.f, 1.f, 0.f)), _rot.z, vec3(0.f, 0.f, 1.f));
	rx = glm::rotate(id, _rot.x, glm::vec3(1.f, 0.f, 0.f));
	ry = glm::rotate(id, _rot.y, glm::vec3(0.f, 1.f, 0.f));
	rz = glm::rotate(id, _rot.z, glm::vec3(0.f, 0.f, 1.f));

	r = rz * ry * rx;
	s = glm::scale(id, _sca);
	mat4 world = p * r * s;
	return world;
}


