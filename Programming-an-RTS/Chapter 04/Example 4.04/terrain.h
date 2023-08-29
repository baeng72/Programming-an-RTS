#pragma once
#include <common.h>
#include "heightMap.h"
#include <glm/glm.hpp>

struct TERRAINVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 color;
	TERRAINVertex() {
		position = normal = glm::vec3(0.f);
		color = glm::vec4(0.f);
	}
	TERRAINVertex(glm::vec3& pos, glm::vec4& col) {
		position = pos;
		normal = glm::vec3(0.f);
		color = col;
	}
};

struct PATCH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Mesh::Mesh>	_mesh;
	PATCH();
	~PATCH();
	bool CreateMesh(HEIGHTMAP& hm, Rect source, Renderer::RenderDevice* pdevice, int index);
	void Render(Renderer::Shader* pshader);
	void Release();
};

class TERRAIN {
	friend class APPLICATION;
	INTPOINT _size;
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::vector<PATCH*> _patches;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::unique_ptr<Renderer::Shader> _shader;
	
public:
	TERRAIN();
	void Cleanup();
	void Init(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> manager, INTPOINT size_);
	void GenerateRandomTerrain(int numPatches);
	void CreatePatches(int numPatches);
	void Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light);
	void Release();
	void SetWireframe(bool wireframe);
};