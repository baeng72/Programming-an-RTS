
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

	if (newFocus.x > -50.f && newFocus.x < 50.f && newFocus.z >-50.f && newFocus.z < 50.f)
		_focus = newFocus;
}

void CAMERA::Pitch(float f) {
	_beta += f;
	if (_beta > glm::pi<float>() / 2.f)
		_beta = glm::pi<float>() / 2.f - 0.1f;
	if (_beta < 0.3f)
		_beta = 0.3f;
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

	if (_radius < 5.f)
		_radius = 5.f;
	if (_radius > 100.f)
		_radius = 100.f;
}

void CAMERA::Update(MOUSE&mouse, float delta)
{
	//Restrict focus movement to xz plane
	_right.y = _look.y = 0.f;
	_look = glm::normalize(_look);
	_right = glm::normalize(_right);

	//Move Focus (i.e. scroll)
	if (mouse.x < mouse._viewport.left+10)
		Scroll(-_right * delta * 5.f);
	if (mouse.x > mouse._viewport.right - 10)
		Scroll(_right * delta * 5.f);
	if (mouse.y < mouse._viewport.top + 10)
		Scroll(_look * delta * 5.f);
	if (mouse.y > mouse._viewport.bottom-10)
		Scroll(-_look * delta * 5.f);

	//Move Camera (i.e. Change Angle)
	if (_pwindow->IsKeyPressed(KEY_A))
		Yaw(-delta);
	if (_pwindow->IsKeyPressed(KEY_D))
		Yaw(delta);
	if (_pwindow->IsKeyPressed(KEY_W))
		Pitch(delta);
	if (_pwindow->IsKeyPressed(KEY_S))
		Pitch(-delta);

	//Zoom (i.e. change _fov)
	if (_pwindow->IsKeyPressed(KEY_KP_ADD))
		Zoom(-delta);
	if (_pwindow->IsKeyPressed(KEY_KP_SUBTRACT))
		Zoom(delta);

	//change _radius
	if (_pwindow->IsKeyPressed(KEY_KP_ADD))
		ChangeRadius(-delta * 100.f);
	if (_pwindow->IsKeyPressed(KEY_KP_SUBTRACT))
		ChangeRadius(delta * 100.f);
	
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

	//keep basis vectors orthogonal
	_right.x = matView[0][0];
	_right.y = matView[1][0];
	_right.z = matView[2][0];
	_right = glm::normalize(_right);

	_look.x = matView[0][2];
	_look.y = matView[1][2];
	_look.z = matView[2][2];
	_look = glm::normalize(_look);

	return matView;
}

glm::mat4 CAMERA::GetProjectionMatrix()
{
	int width, height;
	_pwindow->GetWindowSize(width, height);	
	glm::mat4 matProj = Core::perspective(quarterpi, (float)width, (float)height, 1.f, 1000.f);
	return matProj;
}

