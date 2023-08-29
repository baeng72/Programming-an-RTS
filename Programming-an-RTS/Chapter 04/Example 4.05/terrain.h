#pragma once
#include <common.h>
#include "heightMap.h"


struct TERRAINVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	TERRAINVertex() {
		position = normal = glm::vec3(0.f);
		uv = glm::vec2(0.f);
	}
	TERRAINVertex(glm::vec3& pos, glm::vec2& tex) {
		position = pos;
		normal = glm::vec3(0.f,1.f,0.f);
		uv = tex;
	}
};

struct PATCH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Mesh::Mesh>	_mesh;
	std::unique_ptr<Renderer::Shader> _shader;
	std::unique_ptr<Renderer::Buffer> _attrBuffer;
	PATCH();
	~PATCH();
	bool CreateMesh(HEIGHTMAP& hm, Rect source, Renderer::RenderDevice* pdevice, void*shaderData,std::vector<std::unique_ptr<Renderer::Texture>>&textures);
	void Render(glm::mat4&mat4,glm::mat4&world, Renderer::DirectionalLight& light);
	void Release();
	void SetWireframe(bool wireframe) { _shader->SetWireframe(wireframe); }
};

class TERRAIN {
	friend class APPLICATION;
	INTPOINT _size;
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::vector<PATCH*> _patches;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	//std::unique_ptr<Renderer::Shader> _shader;
	//std::unique_ptr<Renderer::Buffer> _colorBuffer;
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