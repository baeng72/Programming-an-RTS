#pragma once
#include "../../../common/common.h"
#include "../../../common/Renderer/RenderDevice.h"
#include "../../../common/Renderer/ShaderManager.h"
#include "../../../common/Renderer/Shader.h"
#include "../../../common/Renderer/Mesh.h"
#include "heightMap.h"
#include "object.h"
#include <glm/glm.hpp>

struct TERRAINVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec2 uvAlpha;
	TERRAINVertex() {
		position = normal = glm::vec3(0.f);
		uv = glm::vec2(0.f);
		uvAlpha = glm::vec2(0.f);
	}
	TERRAINVertex(glm::vec3& pos, glm::vec2& tex,glm::vec2& tex2) {
		position = pos;
		normal = glm::vec3(0.f,1.f,0.f);
		uv = tex;
		uvAlpha = tex2;
	}
};

struct PATCH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Renderer::Mesh>	_mesh;
	PATCH();
	~PATCH();
	bool CreateMesh(HEIGHTMAP& hm, Rect source, Renderer::RenderDevice* pdevice);
	void Render(Renderer::Shader * pshader);
	void Release();	
};

class TERRAIN {
	friend class APPLICATION;
	INTPOINT _size;
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::vector<PATCH*> _patches;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::vector<std::unique_ptr<Renderer::Texture>> _diffuseMaps;
	std::unique_ptr<Renderer::Texture> _alphaMap;
	std::unique_ptr<Renderer::Shader> _shader;
	std::vector<OBJECT> _objects;
	
public:
	TERRAIN();
	void Cleanup();
	void Init(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> manager, INTPOINT size_);
	void GenerateRandomTerrain(int numPatches);
	void CreatePatches(int numPatches);
	void CalculateAlphaMaps();
	void Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light);
	void Release();
	void SetWireframe(bool wireframe);
	void AddObject(int type, INTPOINT mappos);
};