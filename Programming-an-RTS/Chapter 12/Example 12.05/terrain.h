#pragma once
#define __USE__PROFILER__
#include "heightMap.h"
#include "object.h"
#include "mouse.h"

class MAPOBJECT;
class CAMERA;
class PLAYER;

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
	bool _visible;
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
	MAPOBJECT* _pMapObject;
	MAPTILE() {
		_type = _set = 0;
		_height = _cost = 0.f;
		_walkable = false;
		for (int i = 0; i < numNeighbors; i++) {
			_neighbors[i] = nullptr;
		}
		_pParent = nullptr;
		_pMapObject = nullptr;
	}
};

class TERRAIN {
	friend struct PATCH;
	friend class MOUSE;
	friend class APPLICATION;
	friend class CAMERA;
	friend class UNIT;
	
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<HEIGHTMAP> _heightMap;
	std::vector<PATCH*> _patches;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::vector<std::unique_ptr<Renderer::Texture>> _diffuseMaps;
	std::unique_ptr<Renderer::Texture> _alphaMap;
	std::unique_ptr<Renderer::Texture> _lightMap;
	
	std::unique_ptr<Renderer::Shader> _shader;
	std::unique_ptr<Renderer::Shader> _objectShader;
	std::unique_ptr<Renderer::Font> _font;
	std::vector<OBJECT> _objects;
	
	vec3 _dirToSun;

	////Fog of war variables
	////sight/visible pass
	//std::unique_ptr<Renderer::Texture> _sightTexture;
	//std::unique_ptr<Mesh::Mesh> _sightMesh;
	//std::unique_ptr<Renderer::Texture> _visibleTexture;
	//std::unique_ptr<Renderer::FrameBuffer> _visibleFramebuffer;
	//std::unique_ptr<Renderer::Shader> _visibleShader;
	////visited pass
	//std::vector<std::unique_ptr<Renderer::Texture>> _visitedTextures;
	//std::unique_ptr<Renderer::FrameBuffer> _visitedFramebuffer;
	//std::unique_ptr<Renderer::Shader> _visitedShader;
	////fog pass
	//std::unique_ptr<Renderer::Texture> _fowTexture;
	//std::unique_ptr<Renderer::FrameBuffer> _fowFramebuffer;
	//std::unique_ptr<Renderer::Shader> _fowShader;
	////Landscape 
	//std::unique_ptr<Renderer::Texture> _whiteLightmap;
	//std::unique_ptr<Renderer::Texture> _landscapeTexture;
	//std::unique_ptr<Renderer::Texture> _landscapeDepthMap;
	//std::unique_ptr<Renderer::FrameBuffer> _landscapeFramebuffer;
	////Minimap
	//std::unique_ptr<Renderer::Texture> _minimapBorder;
	//std::unique_ptr<Renderer::Texture> _minimapTexture;
	//std::unique_ptr<Renderer::FrameBuffer> _minimapFramebuffer;
	//std::unique_ptr<Renderer::Shader> _minimapShader;
	//std::unique_ptr<Renderer::Sprite> _minimapSprite;

	//bool _firstFogOfWar;
	//bool _fogOverride;
	//void InitFogOfWar();
	//void InitLandscape();
	//void InitMinimap();
	
public:
	
	MAPTILE* _pMaptiles;
	std::vector<uint8_t> _visibleTiles;
	std::vector<uint8_t> _visitedTiles;
	bool _updateSight;
	INTPOINT _size;
public:
	TERRAIN();
	void Cleanup();
	void Init(Renderer::RenderDevice* pdevice,Core::Window*pwindow, std::shared_ptr<Renderer::ShaderManager> manager, INTPOINT size_);
	void GenerateRandomTerrain(Core::Window*pwindow,int numPatches);
	void CreatePatches(int numPatches);
	void CalculateAlphaMaps();
	void CalculateLightMap(Core::Window*pwindow);
	void Render(mat4&viewProj, mat4&model, Renderer::DirectionalLight&light,CAMERA&camera);
	void Progress(const char* ptext, float prc);
	vec3 GetNormal(int x, int y);
	void Release();
	void SetWireframe(bool wireframe);
	void AddObject(int type, INTPOINT mappos);
	//pathfinding
	bool Within(INTPOINT p);	//test if a point is within the bounds of the terrain
	void InitPathfinding();
	void UpdatePathfinding(Rect* r);
	void CreateTileSets();
	std::vector<INTPOINT> GetPath(INTPOINT start, INTPOINT goal,bool considerUnits);
	MAPTILE* GetTile(int x, int y);
	MAPTILE* GetTile(INTPOINT p) {		return GetTile(p.x, p.y);	}
	vec3 GetWorldPos(INTPOINT mappos);

	//Save and Load Map
	void SaveTerrain(const char* pfilename);
	void LoadTerrain(const char* pfilename);	

	//void UpdateSightMatrixes(std::vector<MAPOBJECT*>& mapObjects);
	//
	//void RenderFogOfWar(PLAYER*player);
	//void toggleFogOfWar(){ _fogOverride = !_fogOverride; }

	//
	//Renderer::Texture* GetLandscapeTexture()const { return _landscapeTexture.get(); }//make minimap part of terrain?
	//void RenderLandscape();//called when Generate Random Terrain is called?
	//void UpdateMinimap(std::vector<PLAYER*>& players);
	//void RenderMinimap(CAMERA& camera,MOUSE&mouse,Rect&r,Renderer::Line2D*pline);



};