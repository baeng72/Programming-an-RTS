#include "object.h"
#include "functions.h"

std::vector<MESH*> objectMeshes;




bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* tile = new MESH(pdevice, shaderManager, Core::ResourcePath::GetProjectResourcePath("Objects/tile.x"));
	objectMeshes.push_back(tile);

	MESH* house = new MESH(pdevice, shaderManager, Core::ResourcePath::GetProjectResourcePath("Objects/house.x"));
	objectMeshes.push_back(house);

	MESH* house2 = new MESH(pdevice, shaderManager, Core::ResourcePath::GetProjectResourcePath("Objects/house2.x"));
	objectMeshes.push_back(house2);

	MESH* park = new MESH(pdevice, shaderManager, Core::ResourcePath::GetProjectResourcePath("Objects/park.x"));
	objectMeshes.push_back(park);
	

	

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
	_rendered = false;
}

OBJECT::OBJECT(int t, vec3 pos, vec3 rot, vec3 sca) {
	_type = t;
	
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(sca);
	_meshInstance.SetMesh(objectMeshes[t]);
	
	_BBox = _meshInstance.GetBoundingBox();
	_BSphere = _meshInstance.GetBoundingSphere();
}

void OBJECT::Render(glm::mat4& viewProj, Renderer::DirectionalLight& light) {
	_meshInstance.Render(viewProj, light);//TODO: simple optimization, add function to shader manager to set ubo once, instance of setting for each shader instance
}

