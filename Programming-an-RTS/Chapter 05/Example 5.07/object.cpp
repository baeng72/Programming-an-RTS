#include "object.h"


std::vector<MESH*> objectMeshes;




bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* mech1 = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.07/Objects/mech1.x");
	objectMeshes.push_back(mech1);

	MESH* mech2 = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.07/Objects/mech2.x");
	objectMeshes.push_back(mech2);

	MESH* mech3 = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.07/Objects/mech3.x");
	objectMeshes.push_back(mech3);

	
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

OBJECT::OBJECT(int t, vec3 pos, vec3 rot) {
	_type = t;
	
	for (int i = 0; i < 3; i++) {
		_meshInstances[i].SetPosition(pos);
		_meshInstances[i].SetRotation(rot);
		_meshInstances[i].SetScale(vec3(1.f));
		_meshInstances[i].SetMesh(objectMeshes[t+i]);
	}
	_BBox = _meshInstances[1].GetBoundingBox();
	_BSphere = _meshInstances[1].GetBoundingSphere();
}

void OBJECT::Render(CAMERA*camera,glm::mat4& viewProj, Renderer::DirectionalLight& light,long&noFaces, int &noObjects) {
	if (camera == nullptr) {
		//render hi-res mesh
		_meshInstances[0].Render(viewProj, light);
		
		noFaces += (long)(_meshInstances[0]._mesh->_indices.size() / 3);
		noObjects++;
	}
	else {
		if (!camera->Cull(_BBox)) {//cull objects

			//Distance from objects to camera
			float dist = glm::length(_meshInstances[0]._pos - camera->_eye);
			noObjects++;

			if (dist < 50.f) {	//Render high res
				_meshInstances[0].Render(viewProj, light);
				noFaces += (long)_meshInstances[0]._mesh->_indices.size() / 3;
			}
			else if (dist < 100.f) {	//Average distance from camera
				_meshInstances[1].Render(viewProj, light);
				noFaces += (long)_meshInstances[1]._mesh->_indices.size() / 3;
			}
			else {	//far from camera
				_meshInstances[2].Render(viewProj, light);
				noFaces += (long)_meshInstances[2]._mesh->_indices.size() / 3;
			}
		}
	}
	
}

