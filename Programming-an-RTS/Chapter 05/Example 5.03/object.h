#pragma once

#include <common.h>
#include "Mesh.h"
#include "intpoint.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshaderManager);
bool UnloadObjectResources();
void ObjectSetWireframe(bool wireframe);

#define DRAGON 0
#define BOB 1
#define RING 2

class OBJECT {
public:
	MESHINSTANCE _meshInstance;
	int _type;
	std::string _name;
	BBOX _BBox;
	BSPHERE _BSphere;

	OBJECT();
	OBJECT(int t,glm::vec3 pos, glm::vec3 rot, glm::vec3 sca);
	void Render(glm::mat4& viewProj, Renderer::DirectionalLight& light);
	void RenderBoundingVolume(int type, mat4& matViewProj, Renderer::DirectionalLight& light);
};