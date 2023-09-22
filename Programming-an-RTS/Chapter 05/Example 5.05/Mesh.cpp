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
	//This may fail on some systems.
	//A pushconstant is guaranteed to be 128bytes, but may be bigger on some systems.
	struct PushConst {
		mat4 viewProj;
		mat4 world;		
		vec4 clrdiffuse;
		vec4 clrspecular;
	};
	struct UBO { Renderer::DirectionalLight light; } ubo = { light };
	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	_shader->Bind();
	for (size_t i = 0; i < _meshes.size(); i++) {
		PushConst pushConst{ matViewProj, matWorld * _xforms[i],_materials[i].diffuse,_materials[i].specular };
		_shader->SetPushConstData(&pushConst, sizeof(pushConst));
		_meshes[i]->Render();
	}
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	_meshes.clear();
}

bool MESH::Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shaderManager, const char* pfilename) {
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));
	uint32_t meshCount = model->GetMeshCount();
	_meshes.resize(meshCount);
	_xforms.resize(meshCount);
	for (uint32_t i = 0; i < meshCount; i++) {
		_meshes[i] = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::position_normal, i));
		_xforms[i] = model->GetMeshXForm(i);

	}
	uint32_t matCount = model->GetMaterialCount();
	_materials.resize(matCount);
	for (uint32_t i = 0; i < matCount; i++) {
		_materials[i] = *model->GetMaterial(i);
	}
	
	
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.05/shaders/mesh.glsl")));	
	
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


