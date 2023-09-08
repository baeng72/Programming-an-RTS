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
	struct UBO {
		Renderer::DirectionalLight light;
	}ubo = { light };
	
	int uboid = 0;


	struct PushConst {
		mat4 matViewProj;
		mat4 matWorld;
	}pushConst = { matViewProj,matWorld * _xform };

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
	_texture = std::unique_ptr<Renderer::Texture>(model->GetTexture(model->GetMeshMaterialIndex(0), Mesh::TextureType::diffuse, 0));
	_xform = model->GetMeshXForm(0);
	uint32_t stride = 0;
	uint32_t count = 0;
	float *ptr = model->GetMeshRawVertices(0, stride, count);
	_vertices.resize(count);
	for (uint32_t i = 0; i < count; i++) {

		_vertices[i].x = ptr[i * stride/sizeof(float)+0];
		_vertices[i].y = ptr[i * stride / sizeof(float) + 1];
		_vertices[i].z = ptr[i * stride / sizeof(float) + 2];
	}
	auto inds = model->GetMeshRawIndices(0, count);
	_indices.resize(count);
	for (uint32_t i = 0; i < count; i++) {
		_indices[i] = inds[i];
	}
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.06/shaders/mesh.glsl")));
	int texid = 0;
	std::vector<Renderer::Texture*> textures = { _texture.get() };
	_shader->SetTexture(texid, textures.data(), 1);
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
		
		/*glm::mat4 p, rx,ry,rz,r, s;
		glm::mat4 identity = glm::mat4(1.f);
		p = glm::translate(identity, _pos);
		rx = glm::rotate(identity, _rot.x, glm::vec3(1.f, 0.f, 0.f));
		ry = glm::rotate(identity, _rot.y, glm::vec3(0.f, 1.f, 0.f));
		rz = glm::rotate(identity, _rot.z, glm::vec3(0.f, 0.f, 1.f));

		r = rz * ry * rx;
		s = glm::scale(identity, _sca);
		glm::mat4 world = p * r * s;*/
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

BBOX MESHINSTANCE::GetBoundingBox()
{
	if(_mesh==nullptr||_mesh->_vertices.size()==0)		
		return BBOX();
	BBOX bBox(vec3(-10000.f), vec3(10000.f));
	mat4 world = GetWorldMatrix() * _mesh->_xform;
	for (size_t i = 0; i < _mesh->_vertices.size(); i++) {
		vec3 pos = _mesh->_vertices[i];
		vec3 xpos = vec3(world * vec4(pos, 1.f));
		//Check if vertex is outside the bounds 
		//if so, then update bounding volume
		bBox.min.x = std::min(bBox.min.x, xpos.x);
		bBox.min.y = std::min(bBox.min.y, xpos.y);
		bBox.min.z = std::min(bBox.min.z, xpos.z);
		bBox.max.x = std::max(bBox.max.x, xpos.x);
		bBox.max.y = std::max(bBox.max.y, xpos.y);
		bBox.max.z = std::max(bBox.max.z, xpos.z);
	}
	bBox.min = bBox.min;
	bBox.max = bBox.max;
	return bBox;
}

BSPHERE MESHINSTANCE::GetBoundingSphere()
{
	if (_mesh == nullptr || _mesh->_vertices.size() == 0)
		return BSPHERE();
	BBOX bBox = GetBoundingBox();
	BSPHERE bSphere;
	mat4 world = GetWorldMatrix() * _mesh->_xform;
	bSphere.center = (bBox.max + bBox.min) / 2.f;//midpoint
	bSphere.radius = 0.f;
	for (size_t i = 0; i < _mesh->_vertices.size(); i++) {
		vec3 pos = _mesh->_vertices[i];
		vec3 xpos = vec3(world * vec4(pos, 1.f));
		float l = glm::length((xpos - bSphere.center));
		bSphere.radius = std::max(bSphere.radius, l);
	}
	
	return bSphere;
}
