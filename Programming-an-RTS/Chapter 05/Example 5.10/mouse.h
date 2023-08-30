#pragma once
#include <common.h>
#include "intpoint.h"
#include "Mesh.h"
#include "terrain.h"

struct RAY {
	vec3 org;
	vec3 dir;
	RAY();
	RAY(vec3 o, vec3 d);
	//our different intersection tests
	float Intersect(MESHINSTANCE iMesh);
	float Intersect(BBOX bBox);
	float Intersect(BSPHERE bSphere);
	float Intersect(MESH *pMesh);
	float Intersect(std::vector<vec3>& vertices, std::vector<uint32_t>& indices);
	float Intersect(std::vector<vec3>& vertices, std::vector<uint32_t>& indices,uint32_t&face,vec2&bary);
};


class MOUSE : public INTPOINT {
	friend class CAMERA;
	Renderer::RenderDevice* _pdevice;
	Window* _pwindow;
	Rect _viewport;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	std::unique_ptr<Renderer::Sprite> _sprite;
	std::unique_ptr<Mesh::Mesh> _sphereMesh;
	std::unique_ptr<Renderer::Shader> _sphereShader;
	float yscroll;
public:

	float _speed;
	int		_type;
	INTPOINT _mappos;
	vec3 _ballPos;
	vec2 _uv;
public:
	MOUSE();
	~MOUSE();
	void Init(Renderer::RenderDevice*pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager,Window* pwindow);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(Rect& dst);
	bool PressInRect(Rect& dst);
	void Update(TERRAIN&terrain);
	void Paint(mat4&matVP,Renderer::DirectionalLight&light);
	RAY GetRay(mat4& matProj, mat4& matView, mat4& matWorld);
	void CalculateMappos(mat4&matProj,mat4&matView,TERRAIN& terrain);
};