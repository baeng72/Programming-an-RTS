#pragma once

#include <common.h>
#include "Mesh.h"
#include "intpoint.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice);
bool UnloadObjectResources();
//void ObjectSetWireframe(bool wireframe);

#define OBJ_TREE 0
#define OBJ_STONE 1

class OBJECT {
	friend class TERRAIN;
	INTPOINT _mappos;
	MESHINSTANCE _meshInstance;
	int _type;
	BBOX _BBox;
	
public:
	OBJECT();
	OBJECT(int t,INTPOINT mp, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca);
	void Render(Renderer::Shader*pshader);
	size_t GetHash()const { return _meshInstance.GetHash(); }
};