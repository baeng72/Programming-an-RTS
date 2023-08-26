#pragma once
#include <common.h>


class CAMERA {
public:
	Window* _pwindow;
	float _fov;
	glm::vec3 _eye;
	glm::vec3 _focus;
	
public:
	CAMERA();

	void Init(Window* pwindow);

	
	//Calculate matrices
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
};
