#include "object.h"

std::vector<MESH*> objectMeshes;

std::vector<std::unique_ptr<Mesh::Mesh>> shapeMeshes;

std::unique_ptr<Renderer::Shader> shapeShader;

bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* dragon = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.03/objects/dragon.x");
	objectMeshes.push_back(dragon);
	MESH* f1 = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.03/objects/footman01.x");
	objectMeshes.push_back(f1);
	MESH* ring = new MESH(pdevice, shaderManager, "../../../../Resources/Chapter 05/Example 5.03/objects/ring.x");
	objectMeshes.push_back(ring);

	std::unique_ptr<Mesh::Shape> shape;
	shape.reset(Mesh::Shape::Create(pdevice));
	shapeMeshes.push_back(std::unique_ptr<Mesh::Mesh>(shape->CreateCube(1.f)));
	shapeMeshes.push_back(std::unique_ptr<Mesh::Mesh>(shape->CreateSphere(1.f, 12, 12)));
	

	shapeShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 05/Example 5.03/shaders/shape.glsl",false)));

	return true;
}

bool UnloadObjectResources() {
	for (auto& mesh : objectMeshes) {
		delete mesh;
	}
	objectMeshes.clear();

	shapeMeshes.clear();

	shapeShader.reset();
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

OBJECT::OBJECT(int t, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) {
	_type = t;
	
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(sca);
	_meshInstance.SetMesh(objectMeshes[t]);

	if (_type == DRAGON)
		_name = "Dragon";
	if (_type == BOB)
		_name = "Bob";
	if (_type == RING)
		_name = "";

	_BBox = _meshInstance.GetBoundingBox();
	_BSphere = _meshInstance.GetBoundingSphere();
}

void OBJECT::Render(glm::mat4& viewProj, Renderer::DirectionalLight& light) {
	_meshInstance.Render(viewProj, light);//TODO: simple optimization, add function to shader manager to set ubo once, instance of setting for each shader instance
}

void OBJECT::RenderBoundingVolume(int type,mat4 &matViewProj,Renderer::DirectionalLight&light)
{
	switch (type) {
	case 0:
	{
		break;
	}
	case 1:
	{
		vec3 center = (_BBox.max + _BBox.min) * 0.5f;
		vec3 size = (_BBox.max - _BBox.min);
		mat4 id = glm::mat4(1.f);//identity
		mat4 scale = glm::scale(id, size);
		mat4 trans = glm::translate(id, center);
		mat4 world = trans * scale;
		Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
		int uboid = 0;


		struct PushConst {
			mat4 world;
			Color color;
		}pushConst = { world,Color(0.f,1.f,0.f,0.5f) };
		shapeShader->SetUniformData(uboid, &ubo, sizeof(ubo));
		shapeShader->SetPushConstData(&pushConst, sizeof(pushConst));
		shapeShader->Bind();
		shapeMeshes[type - 1]->Render();
	}
		break;

	case 2: {
		vec3 center = _BSphere.center;
		mat4 id = glm::mat4(1.f);//identity
		mat4 scale = glm::scale(id, vec3(_BSphere.radius));
		mat4 trans = glm::translate(id, center);
		mat4 world = trans * scale;
		Renderer::FlatShaderDirectionalUBO ubo = { matViewProj,light };
		int uboid = 0;


		struct PushConst {
			mat4 world;
			Color color;
		}pushConst = { world,Color(0.f,1.f,0.f,0.5f) };
		shapeShader->SetUniformData(uboid, &ubo, sizeof(ubo));
		shapeShader->SetPushConstData(&pushConst, sizeof(pushConst));
		shapeShader->Bind();
		shapeMeshes[type-1]->Render();
	}
		break;
	}
	

	
}

