#include "object.h"

std::vector<MESH*> objectMeshes;

bool LoadObjectResources(Renderer::RenderDevice* pdevice ){
	MESH* tree = new MESH(pdevice,  "../../../../Resources/Chapter 09/Example 9.03/meshes/tree.x");
	objectMeshes.push_back(tree);
	MESH* stone = new MESH(pdevice, "../../../../Resources/Chapter 09/Example 9.03/meshes/stone.x");
	objectMeshes.push_back(stone);
	return true;
}

bool UnloadObjectResources() {
	for (auto& mesh : objectMeshes) {
		delete mesh;
	}
	objectMeshes.clear();
	return true;
}



//////////////////////////////////////////////////////
///			OBJECT class
/////////////////////////////////////////////////////

OBJECT::OBJECT() {
	_type = 0;
}

OBJECT::OBJECT(int t,INTPOINT mp, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) {
	_type = t;
	_mappos = mp;
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(sca);
	_meshInstance.SetMesh(objectMeshes[t]);

	_BBox = _meshInstance.GetBoundingBox();
}

void OBJECT::Render(Renderer::Shader*pshader) {
	mat4 matWorld = _meshInstance.GetWorldMatrix();
	mat4 xform = _meshInstance.GetMeshXForm();
	struct PushConst {
		mat4 world;		
	}pushConst = { matWorld * xform};
	pshader->SetPushConstData(&pushConst, sizeof(pushConst));
	_meshInstance.Render(pshader);
}