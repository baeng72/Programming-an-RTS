#pragma once
#include <common.h>

struct BBOX {
	vec3 max;
	vec3 min;
	BBOX() {
		max = vec3(-10000.f);
		min = vec3(10000.f);
	}
	BBOX(vec3 max_, vec3 min_) :max(max_), min(min_) {

	}
};

struct BSPHERE {
	vec3 center;
	float radius;
	BSPHERE() {
		center = vec3(0.f);
		radius = 1.f;
	}
	BSPHERE(vec3 center_, float radius_) :center(center_), radius(radius_) {

	}
};

class MESH {
	friend class OBJECT;
	friend struct RAY;
	friend class MESHINSTANCE;
	Renderer::RenderDevice* _pdevice;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::vector<std::unique_ptr<Mesh::Mesh>> _meshes;
	std::vector<Mesh::ModelMaterial> _materials;
	std::unique_ptr<Renderer::Shader> _shader;
	std::vector<mat4> _xforms;
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
	friend class OBJECT;
	friend struct RAY;
	MESH* _mesh;
public:
	glm::vec3 _pos;
	glm::vec3 _rot;
	glm::vec3 _sca;
public:
	MESHINSTANCE();
	MESHINSTANCE(MESH* meshPtr);
	void Render(glm::mat4& viewProj, Renderer::DirectionalLight& light);
	void SetMesh(MESH* meshPtr) { _mesh = meshPtr; }
	void SetPosition(glm::vec3 p) { _pos = p; }
	void SetRotation(glm::vec3 r) { _rot = r; }
	void SetScale(glm::vec3 s) { _sca = s; }
	
	mat4 GetWorldMatrix();
	BBOX GetBoundingBox();
	BSPHERE GetBoundingSphere();
};