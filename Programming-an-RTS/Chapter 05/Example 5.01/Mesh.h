#pragma once
#include <common.h>

class MESH {
	friend class MESHINSTANCE;
	Renderer::RenderDevice* _pdevice;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::unique_ptr<Mesh::Mesh> _mesh;
	std::unique_ptr<Renderer::Texture> _texture;
	
	std::unique_ptr<Renderer::Shader> _shader;
	glm::mat4 _xform;
	void LoadShader();
public:
	MESH();
	MESH( Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshadermanager, const char* pName);
	~MESH();
	bool Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shadermanager, const char* pName );
	void Render(glm::mat4& matViewProj, glm::mat4& matWorld,Renderer::DirectionalLight&light);
	void Release();
	void SetWireframe(bool wireframe) { _shader->SetWireframe(wireframe); }
};

class MESHINSTANCE {
	MESH* _mesh;
public:
	glm::vec3 _pos;
	glm::vec3 _rot;
	glm::vec3 _sca;
public:
	MESHINSTANCE();
	MESHINSTANCE(MESH* meshPtr);
	void Render(glm::mat4&viewProj,Renderer::DirectionalLight&light);
	void SetMesh(MESH* meshPtr) { _mesh = meshPtr; }
	void SetPosition(glm::vec3 p) { _pos = p; }
	void SetRotation(glm::vec3 r) { _rot = r; }
	void SetScale(glm::vec3 s) { _sca = s; }
};