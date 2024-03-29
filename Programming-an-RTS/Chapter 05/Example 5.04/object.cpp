#include "object.h"
#include "functions.h"

std::vector<MESH*> objectMeshes;

std::vector<std::unique_ptr<Mesh::Mesh>> shapeMeshes;

std::unique_ptr<Renderer::Shader> shapeShader;

std::unique_ptr<Renderer::Line2D> line;


bool LoadObjectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager) {
	MESH* gnome = new MESH(pdevice, shaderManager, Core::ResourcePath::GetProjectResourcePath("units/warrior_gnome.x"));
	objectMeshes.push_back(gnome);
	
	std::unique_ptr<Mesh::Shape> shape;
	shape.reset(Mesh::Shape::Create(pdevice));
	shapeMeshes.push_back(std::unique_ptr<Mesh::Mesh>(shape->CreateCube(1.f)));
	shapeMeshes.push_back(std::unique_ptr<Mesh::Mesh>(shape->CreateSphere(1.f, 12, 12)));
	
	
	shapeShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("shape.glsl"), Renderer::ShaderCullMode::frontFace)));
	

	line.reset(Renderer::Line2D::Create(pdevice));
	int width, height;
	pdevice->GetDimensions(&width, &height);
	line->Update(width, height);
	return true;
}

bool UnloadObjectResources() {
	for (auto& mesh : objectMeshes) {
		delete mesh;
	}
	objectMeshes.clear();

	shapeMeshes.clear();

	shapeShader.reset();

	line.reset();

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

OBJECT::OBJECT(int t, vec3 pos, vec3 rot, vec3 sca,std::string name) {
	_type = t;
	
	_meshInstance.SetPosition(pos);
	_meshInstance.SetRotation(rot);
	_meshInstance.SetScale(sca);
	_meshInstance.SetMesh(objectMeshes[t]);
	_selected = false;
	_name = name;

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
		
		
		Color clr = Color(0.f, 1.f, 0.f, 0.5f);
		shapeShader->SetUniform("viewProj", &matViewProj);
		shapeShader->SetUniform("model", &world);
		shapeShader->SetUniform("color", &clr);
		shapeShader->SetUniform("light.ambient", &light.ambient);
		shapeShader->SetUniform("light.diffuse", &light.diffuse);
		shapeShader->SetUniform("light.specular", &light.specular);
		shapeShader->SetUniform("light.direction", &light.direction);
		shapeShader->Bind();
		shapeMeshes[type - 1]->Bind();
		shapeMeshes[type - 1]->Render();
	}
		break;

	case 2: {
		
		vec3 center = _BSphere.center;
		mat4 id = glm::mat4(1.f);//identity
		mat4 scale = glm::scale(id, vec3(_BSphere.radius));
		mat4 trans = glm::translate(id, center);
		mat4 world = trans * scale;
		
		Color clr = Color(0.f, 1.f, 0.f, 0.5f);
		shapeShader->SetUniform("viewProj", &matViewProj);
		shapeShader->SetUniform("model", &world);
		shapeShader->SetUniform("color", &clr);
		shapeShader->SetUniform("light.ambient", &light.ambient);
		shapeShader->SetUniform("light.diffuse", &light.diffuse);
		shapeShader->SetUniform("light.specular", &light.specular);
		shapeShader->SetUniform("light.direction", &light.direction);
		shapeShader->Bind();
		shapeMeshes[type - 1]->Bind();
		shapeMeshes[type-1]->Render();
	}
		break;
	}
	

	
}

void OBJECT::PaintSelected(mat4&matVP,vec4&viewport)
{
	if (!_selected)
		return;
	float radius = 0.6f;
	float height = 2.2f;

	//Create 8 offset ponts according to the height and radius of a unit
	vec3 offsets[] = { vec3(-radius,0,radius), vec3(radius,0,radius),
					vec3(-radius,0,-radius),vec3(radius,0,-radius),
					vec3(-radius,height,radius), vec3(radius, height, radius),
					vec3(-radius, height, -radius), vec3(radius, height, -radius)
	};

	//Find the max and min points of these 8 offset poits in screen space
	INTPOINT ptMax(-10000), ptMin(10000);

	for (int i = 0; i < 8; i++) {
		vec3 pos = _meshInstance._pos + offsets[i];
		INTPOINT screenPos = GetScreenPos(pos, matVP, viewport);
		ptMax.x = std::max(ptMax.x, screenPos.x);
		ptMax.y = std::max(ptMax.y, screenPos.y);
		ptMin.x = std::min(ptMin.x, screenPos.x);
		ptMin.y = std::min(ptMin.y, screenPos.y);
	}

	Rect scr = { -20,-20,820,620 };//hard coding of screen dimensions!!!! hacky original code
	scr = { (int)viewport.x - 20,(int)viewport.y - 20,(int)viewport.z + 20,(int)viewport.w + 20 };//since we're passing the viewport size, may as well use it
	//Check that the max and min point is within our viewport boundaries
	if (ptMax.inRect(scr) || ptMin.inRect(scr)) {
		float s = (ptMax.x - ptMin.x) / 3.f;
		if ((ptMax.y - ptMin.y) < (ptMax.x - ptMin.x))
			s = (ptMax.y - ptMin.y) / 3.f;

		vec2 corner1[] = { vec2(ptMin.x,ptMin.y + s),		vec2(ptMin.x, ptMin.y),		vec2(ptMin.x + s, ptMin.y) };
		vec2 corner2[] = { vec2(ptMax.x - s,ptMin.y),		vec2(ptMax.x, ptMin.y),		vec2(ptMax.x, ptMin.y + s) };
		vec2 corner3[] = { vec2(ptMax.x,ptMax.y - s),		vec2(ptMax.x, ptMax.y),		vec2(ptMax.x - s, ptMax.y) };
		vec2 corner4[] = { vec2(ptMin.x + s,ptMax.y),		vec2(ptMin.x, ptMax.y),		vec2(ptMin.x, ptMax.y - s) };

		//Draw the 4 corners
		line->Draw(corner1, 3, vec4(1.f), 2);
		line->Draw(corner2, 3, vec4(1.f), 2);
		line->Draw(corner3, 3, vec4(1.f), 2);
		line->Draw(corner4, 3, vec4(1.f), 2);

	}
}

