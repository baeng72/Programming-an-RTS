#pragma once
#include <common.h>
#include "mouse.h"
#include "terrain.h"

class CAMERA {
	friend class APPLICATION;
	Core::Window* _pwindow;
	float _alpha;
	float _beta;
	float _radius;
	float _fov;
public:
	glm::vec3 _eye;
	glm::vec3 _focus;
	glm::vec3 _right;
	glm::vec3 _look;

	Plane _frustum[6];
public:
	CAMERA();

	void Init(Core::Window* pwindow);

	//Movement
	void Scroll(glm::vec3 vec);		//Move focus
	void Pitch(float f);			//Change B-angle
	void Yaw(float f);				//Change A-angle
	void Zoom(float f);				//change FOV
	void ChangeRadius(float f);

	//Calculate Eye position etc
	void Update(MOUSE&mouse, TERRAIN&terrain,float delta);
	void CalculateFrustum(mat4& projection, mat4& view);
	bool Cull(BBOX& bBox);
	bool Cull(BSPHERE& bSphere);

	//Calculate matrices
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
};
