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
	
};


class MOUSE : public INTPOINT {
	friend class CAMERA;
	Renderer::RenderDevice* _pdevice;
	Core::Window* _pwindow;
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
	void Init(Renderer::RenderDevice*pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager,Core::Window* pwindow);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(Rect& dst);
	bool PressInRect(Rect& dst);
	void Update(TERRAIN&terrain);
	void Paint(mat4&matVP,Renderer::DirectionalLight&light);
	RAY GetRay(glm::mat4& matProj, glm::mat4& matView, glm::mat4& matWorld);
	void CalculateMappos(mat4&matProj,mat4&matView,TERRAIN& terrain);
};