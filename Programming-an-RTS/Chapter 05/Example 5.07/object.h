#pragma once

#include <common.h>
#include "Mesh.h"
#include "intpoint.h"
#include "camera.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshaderManager);
bool UnloadObjectResources();
void ObjectSetWireframe(bool wireframe);

#define MECH 0

class OBJECT {
public:
	MESHINSTANCE _meshInstances[3];
	int _type;
	bool _rendered;
	
	BBOX _BBox;
	BSPHERE _BSphere;

	OBJECT();
	OBJECT(int t,vec3 pos, vec3 rot);
	void Render(CAMERA*cam,mat4& viewProj, Renderer::DirectionalLight& light,long&noFaces,int&noObjects);
	
	
	
};