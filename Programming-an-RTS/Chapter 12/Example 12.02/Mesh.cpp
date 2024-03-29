#pragma once
#include "Mesh.h"



MESH::MESH() :_pdevice(nullptr) {
	
}

MESH::MESH(Renderer::RenderDevice* pdevice, const char* pfilename) : _pdevice(pdevice) {	
	Load(pdevice,  pfilename);
}

MESH::~MESH() {
	Release();
}


void MESH::Render(Renderer::Shader* pshader)
{
	
	_multiMesh->Bind();		
	for (uint32_t i = 0; i < _partCount; i++) {
		pshader->SetUniform("color", _colors[i]);
		_multiMesh->Render(i);
	}
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	
	//_meshes.clear();
	//_textures.clear();
}

bool MESH::Load(Renderer::RenderDevice* pdevice, const char* pfilename) {
	_pdevice = pdevice;
	
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));
	_multiMesh = std::unique_ptr<Mesh::MultiMesh>(model->GetMultiMesh(Mesh::MeshType::position_normal));
	_partCount = _multiMesh->GetPartCount();
	std::vector<vec4> diffuseMats(_partCount);
	for (uint32_t i = 0; i < _partCount; i++) {
		auto matid = _multiMesh->GetMaterialIndex(i);
		auto mat = model->GetMaterial(matid);
		diffuseMats[i] = mat->diffuse;
	}
	_colors = diffuseMats;
	_multiMesh->GetWorldXForm(_xform);
	
	return true;
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



void MESHINSTANCE::Render(Renderer::Shader*pshader)
{
	if (_mesh) {
		mat4 world = GetWorldMatrix() * _mesh->_xform;
		pshader->SetUniform("model", world);
		
		_mesh->Render(pshader);
	}
}

mat4 MESHINSTANCE::GetWorldMatrix()
{
	mat4 p, rx, ry, rz, r, s;
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

BBOX MESHINSTANCE::GetBoundingBox()
{
	BBOX bBox = _mesh->GetBoundingBox();
	mat4 world = GetWorldMatrix() * _mesh->_xform;
	bBox.max = world * vec4(bBox.max, 1.f);
	bBox.min = world * vec4(bBox.min, 1.f);
	return bBox;
}

BSPHERE MESHINSTANCE::GetBoundingSphere()
{
	if (_mesh == nullptr)
		return BSPHERE();
	BSPHERE bSphere = _mesh->GetBoundingSphere();
	mat4 world = GetWorldMatrix() * _mesh->_xform;
	bSphere.center = world * vec4(bSphere.center, 1);
	
	return bSphere;
}
