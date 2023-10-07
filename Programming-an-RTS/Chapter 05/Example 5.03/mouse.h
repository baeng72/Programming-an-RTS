#pragma once
#include <common.h>
#include "intpoint.h"
#include "Mesh.h"

struct RAY {
	vec3 org;
	vec3 dir;
	RAY();
	RAY(vec3 o, vec3 d);
	//our different intersection tests
	float Intersect(MESHINSTANCE iMesh);
	float Intersect(BBOX bBox);
	float Intersect(BSPHERE bSphere);

};


class MOUSE : public INTPOINT {
	friend class CAMERA;
	Renderer::RenderDevice* _pdevice;
	Core::Window* _pwindow;
	Rect _viewport;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	std::unique_ptr<Renderer::Sprite> _sprite;
	float yscroll;
public:
	float _speed;
	int		_type;
public:
	MOUSE();
	~MOUSE();
	void Init(Renderer::RenderDevice*pdevice,Core::Window* pwindow);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(Rect& dst);
	bool PressInRect(Rect& dst);
	void Update();
	void Paint();
	RAY GetRay(glm::mat4& matProj, glm::mat4& matView, glm::mat4& matWorld);
};