
#include "camera.h"


CAMERA::CAMERA() {
	Init(nullptr);
}

void CAMERA::Init(Window*pwindow) {
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



void CAMERA::Update(MOUSE&mouse, TERRAIN&terrain, float delta)
{
	//Restrict focus movement to xz plane
	_right.y = _look.y = 0.f;
	_look = glm::normalize(_look);
	_right = glm::normalize(_right);

	//Move Focus (i.e. scroll)
	if (mouse.x < mouse._viewport.left+10)
		Scroll(-_right * delta * (4.f+_radius*0.2f));
	if (mouse.x > mouse._viewport.right - 10)
		Scroll(_right * delta * (4.f + _radius * 0.2f));
	if (mouse.y < mouse._viewport.top + 10)
		Scroll(_look * delta * (4.f + _radius * 0.2f));
	if (mouse.y > mouse._viewport.bottom-10)
		Scroll(-_look * delta * (4.f + _radius * 0.2f));

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

	//change _radius
	if (mouse.WheelUp())
		ChangeRadius(-1.f);
	if (mouse.WheelDown())
		ChangeRadius(1.f);
	
	//Calculate eye position
	float sideRadius = _radius * cos(_beta);
	float height = _radius * sin(_beta);

	_eye = glm::vec3(_focus.x + sideRadius * cos(_alpha),
					_focus.y + height,
					_focus.z + sideRadius * sin(_alpha));

	//Have the focus follow the terrain heights
	//Find patch that the focus is over
	for (int p = 0; p < terrain._patches.size(); p++) {
		Rect mr = terrain._patches[p]->_mapRect;

		//Focus within patch maprect or not?
		if (_focus.x >= mr.left && _focus.x < mr.right &&
			-_focus.z >= mr.top && -_focus.z < mr.bottom) {
			//Collect only the closest intersections
			vec3 org = vec3(_focus.x, 10000.f, _focus.z);
			vec3 dir = vec3(0.f, -1.f, 0.f);
			uint32_t face;
			vec2 hitUV;
			float dist = terrain._patches[p]->Intersect(org, dir,face,hitUV);
			if (dist > 0.f) {
				_focus.y = 10000.f - dist;
			}
		}
	}
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
	glm::mat4 matProj = glm::perspectiveFovLH_ZO(glm::pi<float>() / 4, (float)width, (float)height, 1.f, 1000.f);
	matProj[1][1] *= -1;//vulkan flip
	return matProj;
}

void CAMERA::CalculateFrustum(mat4& projection, mat4& view) {
	mat4 matVP = projection * view;

	// Left clipping plane
	_frustum[0].a = matVP[0][3] + matVP[0][0];
	_frustum[0].b = matVP[1][3] + matVP[1][0];
	_frustum[0].c = matVP[2][3] + matVP[2][0];
	_frustum[0].d = matVP[3][3] + matVP[3][0];

	// Right clipping plane 
	_frustum[1].a = matVP[0][3] - matVP[0][0];
	_frustum[1].b = matVP[1][3] - matVP[1][0];
	_frustum[1].c = matVP[2][3] - matVP[2][0];
	_frustum[1].d = matVP[3][3] - matVP[3][0];

	// Top clipping plane 
	_frustum[2].a = matVP[0][3] - matVP[0][1];
	_frustum[2].b = matVP[1][3] - matVP[1][1];
	_frustum[2].c = matVP[2][3] - matVP[2][1];
	_frustum[2].d = matVP[3][3] - matVP[3][1];

	// Bottom clipping plane 
	_frustum[3].a = matVP[0][3] - matVP[0][1];
	_frustum[3].b = matVP[1][3] - matVP[1][1];
	_frustum[3].c = matVP[2][3] - matVP[2][1];
	_frustum[3].d = matVP[3][3] - matVP[3][1];

	// Near clipping plane 
	_frustum[4].a = matVP[0][2];
	_frustum[4].b = matVP[1][2];
	_frustum[4].c = matVP[2][2];
	_frustum[4].d = matVP[3][2];

	// Far clipping plane 
	_frustum[5].a = matVP[0][3] - matVP[0][2];
	_frustum[5].b = matVP[1][3] - matVP[1][2];
	_frustum[5].c = matVP[2][3] - matVP[2][2];
	_frustum[5].d = matVP[3][3] - matVP[3][2];

	//Normalize planes
	for (int i = 0; i < 6; i++) {
		vec3 vec = vec3(_frustum[i].a, _frustum[i].b, _frustum[i].c);
		float lengthSq = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
		float recipLength = 1.f / sqrtf(lengthSq);
		_frustum[i].a *= recipLength;
		_frustum[i].b *= recipLength;
		_frustum[i].c *= recipLength;
		_frustum[i].d *= recipLength;
	}
}

bool CAMERA::Cull(BBOX& bBox) {
	//For each plane in the view frustum
	for (int f = 0; f < 6; f++)
	{
		vec3 c1, c2;

		//Find furthest point (n1) & nearest point (n2) to the plane
		if (_frustum[f].a > 0.0f) { 
			c1.x = bBox.max.x; 
			c2.x = bBox.min.x; 
		}
		else { 
			c1.x = bBox.min.x; 
			c2.x = bBox.max.x; 
		}
		if (_frustum[f].b > 0.0f) { 
			c1.y = bBox.max.y; 
			c2.y = bBox.min.y; 
		}
		else { 
			c1.y = bBox.min.y; 
			c2.y = bBox.max.y; 
		}
		if (_frustum[f].c > 0.0f) { 
			c1.z = bBox.max.z; 
			c2.z = bBox.min.z; 
		}
		else { 
			c1.z = bBox.min.z; 
			c2.z = bBox.max.z; 
		}

		float distance1 = _frustum[f].a * c1.x + _frustum[f].b * c1.y +
			_frustum[f].c * c1.z + _frustum[f].d;
		float distance2 = _frustum[f].a * c2.x + _frustum[f].b * c2.y +
			_frustum[f].c * c2.z + _frustum[f].d;

		//If both points are on the negative side of the plane, Cull!
		if (distance1 < 0.0f && distance2 < 0.0f)
			return true;
	}

	//Object is inside the volume
	return false;
}

bool CAMERA::Cull(BSPHERE& bSphere) {
	//For each plane in the view frustum
	for (int f = 0; f < 6; f++)
	{
		float distance = glm::dot(bSphere.center, vec3(_frustum[f].a, _frustum[f].b, _frustum[f].c)) + _frustum[f].d;

		if (distance < -bSphere.radius)
			return true;
	}

	//Object is inside the volume
	return false;
}