#pragma once
#include <common.h>

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
	
public:
	CAMERA();

	void Init(Core::Window* pwindow);

	//Movement
	void Scroll(glm::vec3 vec);		//Move focus
	void Pitch(float f);			//Change B-angle
	void Yaw(float f);				//Change A-angle
	void Zoom(float f);				//change FOV
	void ChangeRadius(float f);
	void Update(float delta);
	
	//Calculate matrices
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
};
