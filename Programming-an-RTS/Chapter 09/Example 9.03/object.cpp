#include "object.h"

std::vector<MESH*> objectMeshes;

bool LoadObjectResources(Renderer::RenderDevice* pdevice ){
	MESH* tree = new MESH(pdevice, Core::ResourcePath::GetMeshPath("tree.x"));
	objectMeshes.push_back(tree);
	MESH* stone = new MESH(pdevice, Core::ResourcePath::GetMeshPath("stone.x"));
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

void OBJECT::Render(Renderer::Shader* pshader) {
	mat4 matWorld = _meshInstance.GetWorldMatrix();
	mat4 xform = _meshInstance.GetMeshXForm();
	mat4 worldxform = matWorld * xform;
	pshader->SetUniform("model", &worldxform);
	//if (Core::GetAPI() == Core::API::Vulkan) {

	//	/*struct PushConst {
	//		mat4 world;
	//	}pushConst = { worldxform };*/
	//	//pshader->SetPushConstData(&pushConst, sizeof(pushConst));
	//	//pshader->SetUniformData("PushConst", &pushConst, sizeof(pushConst));
	//	pshader->SetUniform("model", &worldxform);
	//}
	//else {
	//	pshader->SetUniformData("model", &worldxform, sizeof(mat4));
	//}
	
	_meshInstance.Render(pshader);
}