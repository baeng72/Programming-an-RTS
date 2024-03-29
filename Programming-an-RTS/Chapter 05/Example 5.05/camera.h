#pragma once
#include <common.h>


class CAMERA {
public:
	Core::Window* _pwindow;
	float _fov;
	glm::vec3 _eye;
	glm::vec3 _focus;
	
public:
	CAMERA();

	void Init(Core::Window* pwindow);

	
	//Calculate matrices
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
};
