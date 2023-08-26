#include "object.h"

std::vector<MESH*> objectMeshes;

std::unique_ptr<Renderer::Line2D> line;


bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* f1a = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.05/objects/f1a.x");
	objectMeshes.push_back(f1a);

	MESH* f1b = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.05/objects/f1b.x");
	objectMeshes.push_back(f1b);
	
	
	line.reset(Renderer::Line2D::Create(pdevice));

	return true;
}

bool UnloadObjectResources() {
	for (auto& mesh : objectMeshes) {
		delete mesh;
	}
	objectMeshes.clear();
	
	line.reset();

	return true;
}

void ObjectSetWireframe(bool wireframe)
{
	for (auto& object : objectMeshes) {
		object->SetWireframe(wireframe);
	}
}

//////////////////////////////////////////////////////
///			OBJECT class
/////////////////////////////////////////////////////

OBJECT::OBJECT() {
	_type = 0;
}

OBJECT::OBJECT(Window*pwindow,int t, vec3 pos, vec3 rot, float off) {
	_type = t;
	
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(vec3(1.f));
	_meshInstance.SetMesh(objectMeshes[t]);
	
	_prc = 0.f;
	_speed = 30.f;
	_activeWP = 0;
	_nextWP = _activeWP + 1;
	_offset = off;

	//Add cameras
	_activeCam = 0;
	_cameras.resize(3);
	for (int i = 0; i < 3; i++) {
		
		_cameras[i].Init(pwindow);
	}
	
}

vec2 wayPoints[] = { vec2(0.05f, -1.054f), vec2(76.839f, -3.446f), vec2(85.238f, -4.87f), vec2(92.321f, -9.393f),
						   vec2(95.897f, -16.917f), vec2(95.33f, -25.371f), vec2(93.056f, -33.568f), vec2(66.702f, -105.733f),
						   vec2(61.049f, -112.011f), vec2(54.085f, -116.594f), vec2(46.065f, -119.516f), vec2(37.617f, -120.27f),
						   vec2(-64.766f, -117.602f), vec2(-73.053f, -115.573f), vec2(-80.386f, -111.628f), vec2(-86.758f, -105.98f),
						   vec2(-130.339f, -53.405f), vec2(-135.042f, -46.442f), vec2(-137.235f, -38.218f), vec2(-136.028f, -29.767f),
						   vec2(-131.556f, -22.549f), vec2(-124.52f, -18.613f), vec2(-116.265f, -16.981f), vec2(-107.82f, -18.225f),
						   vec2(-100.507f, -22.037f), vec2(-94.091f, -27.636f), vec2(-60.762f, -66.526f), vec2(-53.834f, -71.231f),
						   vec2(-45.89f, -73.977f), vec2(13.784f, -75.918f), vec2(22.103f, -74.003f), vec2(28.424f, -68.345f),
						   vec2(32.008f, -60.721f), vec2(32.969f, -52.331f), vec2(31.777f, -44.109f), vec2(28.054f, -36.428f),
						   vec2(21.441f, -31.122f), vec2(13.529f, -28.57f), vec2(5.153f, -28.072f), vec2(-3.018f, -30.543f),
						   vec2(-7.975f, -37.213f), vec2(-12.147f, -44.651f), vec2(-17.197f, -50.976f), vec2(-25.131f, -54.126f),
						   vec2(-33.395f, -53.507f), vec2(-41.168f, -50.853f), vec2(-46.317f, -44.045f), vec2(-47.582f, -35.874f),
						   vec2(-47.26f, -27.425f), vec2(-44.497f, -19.349f), vec2(-39.651f, -12.716f), vec2(-32.743f, -7.802f),
						   vec2(-24.947f, -4.325f), vec2(-17.018f, -1.44f), vec2(-8.484f, -1.247f) };

vec3 trackCenter = vec3(-10.0f, 0.0f, -60.0f);

void OBJECT::Update(float delta) {
	float distance = glm::length((wayPoints[_activeWP] - wayPoints[_nextWP]));
	_prc += (delta * _speed) / distance;

	if (_prc > 1.f) {
		_prc -= 1.f;
		_activeWP++;
		if (_activeWP > 54)
			_activeWP = 0;

		_nextWP = _activeWP + 1;
		if (_nextWP > 54)
			_nextWP = 0;
	}

	vec3 a = vec3(wayPoints[_activeWP].x, 0.0f, wayPoints[_activeWP].y);
	vec3 b = vec3(wayPoints[_nextWP].x, 0.0f, wayPoints[_nextWP].y);

	//Linear interpolation between the waypoints
	_meshInstance._pos = a - (a * _prc) + (b * _prc);
	_direction = b - a;
	_direction = glm::normalize(_direction);

	//Offset car from track
	vec3 dirCenter = _meshInstance._pos - trackCenter;
	dirCenter = glm::normalize(dirCenter);
	_meshInstance._pos += dirCenter * _offset;

	//Calculate the rotation angle
	float angle = 0.0f;
	if (wayPoints[_activeWP].x - wayPoints[_nextWP].x != 0.0f)
	{
		float f1 = wayPoints[_nextWP].y - wayPoints[_activeWP].y;
		float f2 = wayPoints[_nextWP].x - wayPoints[_activeWP].x;
		angle = atan(f1 / -f2);
		if (f2 < 0.0f)angle -= glm::pi<float>();
	}

	_meshInstance._rot.y = angle;
}

void OBJECT::UpdateCameras()
{
	//Camera 1:		Driver's Head
	_cameras[0]._eye = _meshInstance._pos + _direction * -4.0f + vec3(0.0f, 1.5f, -1.0f);
	_cameras[0]._focus = _meshInstance._pos + _direction * 1.0f + vec3(0.0f, 1.0f, 0.0f);
	_cameras[0]._fov = glm::pi<float>() * 0.3f;

	//Camera 2:		Track Center
	_cameras[1]._eye = trackCenter + vec3(0.0f, 50.0f, 0.0f);
	_cameras[1]._focus = _meshInstance._pos;
	_cameras[1]._fov = glm::pi<float>() * 0.1f;

	//Camera 3:		In front of the car
	_cameras[2]._eye = _meshInstance._pos + _direction * 5.0f + vec3(0.0f, 0.3f, 0.0f);
	_cameras[2]._focus = _meshInstance._pos + vec3(0.0f, 1.0f, 0.0f);
	_cameras[2]._fov = glm::pi<float>() * 0.2f;

	_matView = _cameras[_activeCam].GetViewMatrix();
	_matProj = _cameras[_activeCam].GetProjectionMatrix();
	
}

void OBJECT::Render(glm::mat4& matVP, Renderer::DirectionalLight& light) {
	
	_meshInstance.Render(matVP, light);
}

