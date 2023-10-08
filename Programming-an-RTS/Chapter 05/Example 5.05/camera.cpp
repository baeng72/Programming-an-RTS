
#include "camera.h"


CAMERA::CAMERA() {
	Init(nullptr);
}

void CAMERA::Init(Core::Window*pwindow) {
	_pwindow = pwindow;
	
	_fov = glm::pi<float>() / 4.f;
	_eye = glm::vec3(50.f);
	_focus = glm::vec3(0.f);
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
	mat4 matProj;
	if (Core::GetAPI() == Core::API::Vulkan) {
		matProj = glm::perspectiveFovLH_ZO(glm::pi<float>() / 4, (float)width, (float)height, 1.f, 1000.f);
		matProj[1][1] *= -1;
	}
	else {
		matProj = glm::perspectiveFovLH_NO(glm::pi<float>() / 4, (float)width, (float)height, 1.f, 1000.f);
	}
	return matProj;
}

