#pragma once

#include <common.h>
#include "Mesh.h"
#include "intpoint.h"

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshaderManager);
bool UnloadObjectResources();
void ObjectSetWireframe(bool wireframe);

#define GNOME 0

class OBJECT {
public:
	MESHINSTANCE _meshInstance;
	int _type;
	bool _selected;
	std::string _name;
	BBOX _BBox;
	BSPHERE _BSphere;

	OBJECT();
	OBJECT(int t,vec3 pos, vec3 rot, vec3 sca,std::string name);
	void Render(mat4& viewProj, Renderer::DirectionalLight& light);
	void RenderBoundingVolume(int type, mat4& matViewProj, Renderer::DirectionalLight& light);
	void PaintSelected(mat4&matVP,vec4&viewport);
	vec3 GetPosition() { return _meshInstance._pos; }
};