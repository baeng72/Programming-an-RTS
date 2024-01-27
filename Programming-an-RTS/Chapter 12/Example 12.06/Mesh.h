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
	std::vector<Renderer::Texture*> _textures;
	std::unique_ptr<Mesh::MultiMesh> _multiMesh;
	uint32_t _partCount;
	glm::mat4 _xform;
public:
	MESH();
	MESH(Renderer::RenderDevice* pdevice,  const char* pName);
	~MESH();
	bool Load(Renderer::RenderDevice* pdevice,  const char* pName);
	void Render(Renderer::Shader*pshader);
	
	void Release();
	BBOX GetBoundingBox() {
		BBOX bBox;
		_multiMesh->GetBoundingBox(bBox.min, bBox.max);
		return bBox;
	}
	BSPHERE GetBoundingSphere() { 
		BSPHERE bSphere;
		_multiMesh->GetBoundingSphere(bSphere.center,bSphere.radius); 
		return bSphere;
	}
	size_t GetHash()const { return _multiMesh->GetHash(); }
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
	void Render(Renderer::Shader*pshader);
	//void Render(glm::mat4& matViewProj, Renderer::DirectionalLight& light, vec4& teamCol, vec4& color);
	void SetMesh(MESH* meshPtr) { _mesh = meshPtr; }
	void SetPosition(glm::vec3 p) { _pos = p; }
	void SetRotation(glm::vec3 r) { _rot = r; }
	void SetScale(glm::vec3 s) { _sca = s; }

	mat4 GetWorldMatrix();
	mat4 GetMeshXForm() { return _mesh->_xform; }
	BBOX GetBoundingBox();
	BSPHERE GetBoundingSphere();
	size_t GetHash()const { return _mesh->GetHash(); }
};