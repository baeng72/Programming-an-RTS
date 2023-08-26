#pragma once

#include <common.h>
#include "Mesh.h"
#include "intpoint.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshaderManager);
bool UnloadObjectResources();
void ObjectSetWireframe(bool wireframe);

#define TILE 0
#define HOUSE 1
#define HOUSE2 2
#define PARK 3

class OBJECT {
public:
	MESHINSTANCE _meshInstance;
	int _type;
	bool _rendered;
	
	BBOX _BBox;
	BSPHERE _BSphere;

	OBJECT();
	OBJECT(int t,vec3 pos, vec3 rot, vec3 sca);
	void Render(mat4& viewProj, Renderer::DirectionalLight& light);
	
	
	vec3 GetPosition() { return _meshInstance._pos; }
};