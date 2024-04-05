
#include "camera.h"


CAMERA::CAMERA() {
	Init(nullptr);
}

void CAMERA::Init(Core::Window*pwindow) {
	_pwindow = pwindow;
	_alpha = _beta = 0.5f;
	_radius = 10.f;
	_fov = glm::pi<float>() / 4.f;
	_eye = glm::vec3(50.f);
	_focus = glm::vec3(0.f);
}

void CAMERA::Scroll(glm::vec3 vec) {
	glm::vec3 newFocus = _focus + vec;

	_focus = newFocus;
}

void CAMERA::Pitch(float f) {
	_beta += f;
	if (_beta > glm::pi<float>() / 2.f)
		_beta = glm::pi<float>() / 2.f - 0.05f;
	if (_beta < 0.5f)
		_beta = 0.5f;
}

void CAMERA::Yaw(float f) {
	_alpha += f;
	if (_alpha > glm::pi<float>() * 2.f)
		_alpha -= glm::pi<float>() * 2.f;
	if (_alpha < -glm::pi<float>() * 2.f)
		_alpha += glm::pi<float>() * 2.f;
}

void CAMERA::Zoom(float f) {
	_fov += f;

	if (_fov < 0.1f)
		_fov = 0.1f;
	if (_fov > glm::pi<float>() / 2.f)
		_fov = glm::pi<float>() / 2.f;
}

void CAMERA::ChangeRadius(float f)
{
	_radius += f;

	if (_radius < 2.f)
		_radius = 2.f;
	if (_radius > 70.f)
		_radius = 70.f;
}



void CAMERA::Update( float delta)
{
	if(_pwindow->IsKeyPressed(KEY_LEFT))
		Yaw(-delta);
	if (_pwindow->IsKeyPressed(KEY_RIGHT))
		Yaw(delta);
	if (_pwindow->IsKeyPressed(KEY_UP))
		Pitch(delta);
	if (_pwindow->IsKeyPressed(KEY_DOWN))
		Pitch(-delta);
		
	
	//Move Camera (i.e. Change Angle)
	if (_pwindow->IsKeyPressed(KEY_LEFT))
		Yaw(-delta);
	if (_pwindow->IsKeyPressed(KEY_RIGHT))
		Yaw(delta);
	if (_pwindow->IsKeyPressed(KEY_UP))
		Pitch(delta);
	if (_pwindow->IsKeyPressed(KEY_DOWN))
		Pitch(-delta);

	//Zoom (i.e. change _fov)
	if (_pwindow->IsKeyPressed(KEY_KP_ADD))
		Zoom(-delta);
	if (_pwindow->IsKeyPressed(KEY_KP_SUBTRACT))
		Zoom(delta);

	
	
	//Calculate eye position
	float sideRadius = _radius * cos(_beta);
	float height = _radius * sin(_beta);

	_eye = glm::vec3(_focus.x + sideRadius * cos(_alpha),
					_focus.y + height,
					_focus.z + sideRadius * sin(_alpha));

	
}

glm::mat4 CAMERA::GetViewMatrix()
{
	glm::mat4 matView = glm::lookAtLH(_eye, _focus, glm::vec3(0.f, 1.f, 0.f));

	
	return matView;
}

glm::mat4 CAMERA::GetProjectionMatrix()
{
	int width, height;
	_pwindow->GetWindowSize(width, height);	
	glm::mat4 matProj = Core::perspective(_fov, (float)width, (float)height, 01.f, 1000.f);
	return matProj;
}

