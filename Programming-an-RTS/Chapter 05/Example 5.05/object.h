#pragma once

#include <common.h>
#include "Mesh.h"
#include "camera.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshaderManager);
bool UnloadObjectResources();
void ObjectSetWireframe(bool wireframe);

class OBJECT {
public:
	MESHINSTANCE _meshInstance;
	int _type;
	//Animation variables
	int _activeWP;
	int _nextWP;
	float _prc;
	float _speed;
	float _offset;
	//Camera variables
	std::vector<CAMERA> _cameras;
	int _activeCam;
	vec3 _direction;
	mat4 _matProj;
	mat4 _matView;
	OBJECT();
	OBJECT(Window*pwindow,int t,vec3 pos, vec3 rot, float off);
	void Render(glm::mat4&matVP,Renderer::DirectionalLight& light);
	void Update(float delta);
	void UpdateCameras();
};