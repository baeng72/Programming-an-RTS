#include "object.h"

std::vector<MESH*> objectMeshes;

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* tree = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.10/meshes/tree.x");
	objectMeshes.push_back(tree);
	MESH* stone = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.10/meshes/stone.x");
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

OBJECT::OBJECT(int t,INTPOINT mp, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) {
	_type = t;
	_mappos = mp;
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(sca);
	_meshInstance.SetMesh(objectMeshes[t]);
}

void OBJECT::Render(glm::mat4& viewProj, Renderer::DirectionalLight& light) {
	_meshInstance.Render(viewProj, light);//TODO: simple optimization, add function to shader manager to set ubo once, instance of setting for each shader instance
}