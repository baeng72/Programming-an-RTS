#pragma once

#include "heightMap.h"
#include "object.h"
#include "mouse.h"

struct TERRAINVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec2 uvAlpha;
	TERRAINVertex() {
		position = normal = vec3(0.f);
		uv = vec2(0.f);
		uvAlpha = vec2(0.f);
	}
	TERRAINVertex(vec3& pos,vec3&norm, vec2& tex,vec2& tex2) {
		position = pos;
		normal = norm;
		uv = tex;
		uvAlpha = tex2;
	}
};

struct PATCH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Mesh::Mesh>	_mesh;
	std::vector<vec3> _vertices;
	std::vector<uint32_t> _indices;
	Rect _mapRect;
	BBOX _BBox;
	PATCH();
	~PATCH();
	bool CreateMesh(TERRAIN&t, Rect source, Renderer::RenderDevice* pdevice);
	void Render();
	void Release();
	

};
constexpr int numNeighbors = 8;
struct MAPTILE {
	int _type;
	int _set;
	float _height;
	float _cost;
	bool _walkable;
	MAPTILE* _neighbors[8];

	//Pathfinding variables
	INTPOINT _mappos;
	float f;
	float g;
	bool open;
	bool closed;
	MAPTILE* _pParent;

	MAPTILE() {
		_type = _set = 0;
		_height = _cost = 0.f;
		_walkable = false;
		for (int i = 0; i < numNeighbors; i++) {
			_neighbors[i] = nullptr;
		}
	}
};

class TERRAIN {
	friend struct PATCH;
	friend class MOUSE;
	friend class APPLICATION;
	friend class CAMERA;
	INTPOINT _size;
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::vector<PATCH*> _patches;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::vector<std::unique_ptr<Renderer::Texture>> _diffuseMaps;
	std::unique_ptr<Renderer::Texture> _alphaMap;
	std::unique_ptr<Renderer::Texture> _lightMap;
	std::unique_ptr<Renderer::Shader> _shader;
	std::unique_ptr<Renderer::Font> _font;
	std::vector<OBJECT> _objects;
	
	vec3 _dirToSun;
public:
	MAPTILE* _pMaptiles;
public:
	TERRAIN();
	void Cleanup();
	void Init(Renderer::RenderDevice* pdevice,Core::Window*pwindow, std::shared_ptr<Renderer::ShaderManager> manager, INTPOINT size_);
	void GenerateRandomTerrain(Core::Window*pwindow,int numPatches);
	void CreatePatches(int numPatches);
	void CalculateAlphaMaps();
	void CalculateLightMap(Core::Window*pwindow);
	void Render(mat4&viewProj, mat4&model, Renderer::DirectionalLight&light);
	void Progress(const char* ptext, float prc);
	vec3 GetNormal(int x, int y);
	void Release();
	void SetWireframe(bool wireframe);
	void AddObject(int type, INTPOINT mappos);
	//pathfinding
	bool Within(INTPOINT p);	//test if a point is within the bounds of the terrain
	void InitPathfinding();
	void CreateTileSets();
	std::vector<INTPOINT> GetPath(INTPOINT start, INTPOINT goal);
	MAPTILE* GetTile(int x, int y);
	MAPTILE* GetTile(INTPOINT p) {		return GetTile(p.x, p.y);	}
	vec3 GetWorldPos(INTPOINT mappos);

	//Save and Load Map
	void SaveTerrain(const char* pfilename);
	void LoadTerrain(const char* pfilename);
};