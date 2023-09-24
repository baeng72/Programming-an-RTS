#pragma once
#include "Mesh.h"
#include <meshoptimizer/src/meshoptimizer.h>


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


	Renderer::FlatShaderPushConst pushConst{matWorld * _xform};

	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	_shader->Bind();
	_mesh->Render();
}
void MESH::RenderProgressive(mat4& matViewProj, mat4& matWorld, Renderer::DirectionalLight& light)
{
	Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
	int uboid = 0;


	Renderer::FlatShaderPushConst pushConst{ matWorld * _xform };

	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	_shader->Bind();
	_progressiveMesh->Render();
}

void MESH::Release()
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	_progressiveMesh.reset();
	_mesh.reset();
}

void MESH::SetNumProgressiveFaces(int numFaces)
{
	/*float resultError = 0;
	int numIndices = numFaces * 3;
	float targetError = 1e-2f;
	uint32_t vertexCount = _progressiveVertices.size() / _vertexStride;
	
	std::vector<uint32_t> lodIndices(_progressiveIndices.size());
	
	lodIndices.resize(meshopt_simplify(lodIndices.data(), _progressiveIndices.data(), _progressiveIndices.size(),&_progressiveVertices[0], vertexCount, _vertexStride, numIndices, 0.1f, 0, &resultError));
	_numFaces = lodIndices.size()/3;
	_progressiveMesh.reset(Mesh::Mesh::Create(_pdevice, _progressiveVertices.data(), (uint32_t)vertexCount*_vertexStride, lodIndices.data(), (uint32_t)lodIndices.size()*sizeof(uint32_t)));*/
	_progressiveMesh->SetIndexCount(numFaces * 3);
}

bool MESH::Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shaderManager, const char* pfilename) {
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, pfilename));
	_mesh = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::position_normal_uv, 0));
	_texture = std::unique_ptr<Renderer::Texture>(model->GetTexture(model->GetMeshMaterialIndex(0),Mesh::TextureType::diffuse, 0));
	_xform = model->GetMeshXForm(0);
	//uint32_t vertStride = 0, vertCount = 0;
	//float *pvertices = model->GetMeshRawVertices(0, vertStride, vertCount);
	//uint32_t indCount = 0;
	//uint32_t*pindices=model->GetMeshRawIndices(0, indCount);
	//std::vector<uint32_t> indices(pindices, pindices + indCount);
	//
	//std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
	//size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, vertStride);
	//_progressiveIndices.resize(indCount);
	//meshopt_remapIndexBuffer(_progressiveIndices.data(), pindices, indCount, remap.data());
	//_progressiveVertices.resize(vertStride * vertCount);
	//meshopt_remapVertexBuffer(_progressiveVertices.data(), pvertices, vertCount, vertStride, remap.data());
	//_progressiveMesh = std::unique_ptr<Mesh::Mesh>(Mesh::Mesh::Create(pdevice, _progressiveVertices.data(), vertCount*vertStride, _progressiveIndices.data(), indCount*sizeof(uint32_t)));
	//_numFaces = indCount/3;
	//_vertexStride = vertStride;
	_progressiveMesh = std::unique_ptr<Mesh::ProgressiveMesh>(model->GetProgressiveMesh(Mesh::MeshType::position_normal_uv, 0));
	LoadShader();
	return true;
}





void MESH::LoadShader()
{
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.08/shaders/mesh.glsl")));
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

void MESHINSTANCE::RenderProgressive(glm::mat4& viewProj, Renderer::DirectionalLight& light)
{
	if (_mesh) {
		glm::mat4 p, rx, ry, rz, r, s;
		glm::mat4 identity = glm::mat4(1.f);
		p = glm::translate(identity, _pos);
		rx = glm::rotate(identity, _rot.x, glm::vec3(1.f, 0.f, 0.f));
		ry = glm::rotate(identity, _rot.y, glm::vec3(0.f, 1.f, 0.f));
		rz = glm::rotate(identity, _rot.z, glm::vec3(0.f, 0.f, 1.f));

		r = rz * ry * rx;
		s = glm::scale(identity, _sca);
		glm::mat4 world = p * r * s;		
		_mesh->RenderProgressive(viewProj, world, light);
	}
}

void MESHINSTANCE::SetLOD(int numFaces)
{
	_mesh->SetNumProgressiveFaces(numFaces);
	
}

long MESHINSTANCE::GetNumProgressiveFaces()
{
	return _mesh->GetProgressiveFaceCount();
}
