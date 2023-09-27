#include "skinnedMesh.h"

SKINNEDMESH::SKINNEDMESH() {

}

SKINNEDMESH::~SKINNEDMESH() {
	_pdevice->Wait();
	_sphereShader.reset();
	_sphere.reset();
}

void SKINNEDMESH::Load(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager, const char* fileName)
{
	_pdevice = pdevice;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, fileName));
	_boneCount = model->GetBoneCount(0);
	model->GetBoneNames(0, _boneNames);
	model->GetBoneXForms(0, _boneXForms);
	model->GetBoneHierarchy(0, _boneHierarchy);

	
	_xform = model->GetMeshXForm(0);

	_line.reset(Renderer::Line2D::Create(pdevice));
	_line->Update(800, 600);
	std::unique_ptr<Mesh::Shape> shape = std::unique_ptr<Mesh::Shape>(Mesh::Shape::Create(pdevice));
	_sphere = std::unique_ptr<Mesh::Mesh>(shape->CreateSphere(0.07f, 12, 12));
	_sphereShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.01/shaders/shape.glsl", false)));
	//premultiply transforms
	mat4 xform = _boneXForms[0];
	for (int i = 1; i < _boneCount; i++) {
		int parentID = _boneHierarchy[i];
		mat4 parentXForm = _boneXForms[parentID];
		xform = parentXForm * _boneXForms[i];
		_boneXForms[i] = xform;
	}
}


void SKINNEDMESH::Update() {
	
}

void SKINNEDMESH::Render(mat4& matVP,mat4&matWorld,Renderer::DirectionalLight&light) {
	glm::mat4 r = glm::rotate(glm::mat4(1.f), -glm::pi<float>() * 0.5f, glm::vec3(0.f, 1.f, 0.f));
	struct UBO {
		mat4 matVP;
		Renderer::DirectionalLight light;
	}ubo = { matVP,light };
	struct PushConst {
		mat4 world;
		Color color;
	}pushConst;
	pushConst.color= Color(0.1f,1.f,0.1f,0.5f) ;
	
	_sphereShader->SetUniformData("UBO", &ubo, sizeof(ubo));
	for (int i = 0; i < _boneCount; i++) {
		int parentID = _boneHierarchy[i];
		mat4 xform = _boneXForms[i];
		pushConst.world = matWorld * xform * r;
		_sphereShader->SetPushConstData(&pushConst, sizeof(pushConst));
		_sphereShader->Bind();
		_sphere->Render();
		if (parentID >= 0) {
			auto& name = _boneNames[i];
			auto& parentName = _boneNames[parentID];
			vec3 curr = xform[3];
			vec3 parent = _boneXForms[parentID][3];
			vec3 vec = (curr - parent);
			float len = glm::length(vec);
			if (len < 2.f) {
				glm::vec3 currProj = glm::project(curr, matWorld, matVP, vec4(0, 0, 800, 600));
				vec3 parentProj = glm::project(parent, matWorld, matVP, vec4(0, 0, 800, 600));
				vec2 verts[2] = { vec2(currProj ), vec2(parentProj)		};
				_line->Draw(verts, 2, Color(1.f, 0.f, 0.f, 1.f));
			}
		}
		
	}
}