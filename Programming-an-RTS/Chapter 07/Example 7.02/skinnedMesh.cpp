#include "skinnedMesh.h"

SKINNEDMESH::SKINNEDMESH() {
	_currAnimation = 0;
	_time = 0.f;
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
	
	_animatedMesh = std::unique_ptr<Mesh::AnimatedMesh>(model->GetAnimatedMesh(Mesh::MeshType::pos_norm_uv_bones, 0));
	//copy animations
	uint32_t animationCount = model->GetAnimationCount(0);
	_animations.resize(animationCount);
	for (uint32_t i = 0; i < animationCount; i++) {
		model->GetAnimation(0, i, _animations[i]);
	}
	_xform = model->GetMeshXForm(0);

	_line.reset(Renderer::Line2D::Create(pdevice));
	_line->Update(800, 600);
	std::unique_ptr<Mesh::Shape> shape = std::unique_ptr<Mesh::Shape>(Mesh::Shape::Create(pdevice));
	_sphere = std::unique_ptr<Mesh::Mesh>(shape->CreateSphere(0.07f, 12, 12));
	_sphereShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.02/shaders/shape.glsl", false)));
	
	
}


void SKINNEDMESH::Update() {
	
}

void SKINNEDMESH::SetPose(float time)
{
	_animatedMesh->SetPose(time*300.f);
	
}

void SKINNEDMESH::SetAnimation(const char* pname)
{
	for (size_t i = 0; i < _animations.size(); i++) {
		if (_animations[i].name == pname) {
			_currAnimation = (uint32_t)i;
			break;
		}
	}
	_animatedMesh->SetAnimation(_currAnimation, false);
	_time = 0.f;
}

std::vector<std::string> SKINNEDMESH::GetAnimations()
{
	std::vector<std::string> animationNames;
	for (auto& ani : _animations) {
		animationNames.push_back(ani.name);
	}
	return animationNames;
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
	std::vector<mat4> poseXForms;
	_animatedMesh->GetPoseXForms(poseXForms);
	
	for (int i = 0; i < _boneCount; i++) {
		int parentID = _boneHierarchy[i];
		mat4 pxform = poseXForms[i];
		pushConst.world = _xform * matWorld * pxform;// *r;
		_sphereShader->SetPushConstData(&pushConst, sizeof(pushConst));
		_sphere->Render(_sphereShader.get());
		if (parentID >= 0) {
	
			mat4 parentXForm = poseXForms[parentID];
			
			auto& name = _boneNames[i];
			auto& parentName = _boneNames[parentID];
			vec3 curr = pxform[3];
			vec3 parent = parentXForm[3];
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