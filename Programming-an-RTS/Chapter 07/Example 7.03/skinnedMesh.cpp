#include "skinnedMesh.h"
#include <cmath>
SKINNEDMESH::SKINNEDMESH() {
	_currAnimation = 0;
	_time = 0.f;
}

SKINNEDMESH::~SKINNEDMESH() {
	_pdevice->Wait();
	_meshShader.reset();
	_mesh.reset();
	
}

void SKINNEDMESH::Load(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager, const char* fileName)
{
	_pdevice = pdevice;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, fileName));
	_meshTexture = std::unique_ptr<Renderer::Texture>(model->GetTexture(model->GetMeshMaterialIndex(0),Mesh::TextureType::diffuse, 0));
	_mesh = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::pos_norm_uv_bones, 0));
	_animatedMesh = std::unique_ptr<Mesh::AnimatedMesh>(model->GetAnimatedMesh(Mesh::MeshType::pos_norm_uv_bones, 0));
	uint32_t animationCount = model->GetAnimationCount(0);
	_animations.resize(animationCount);
	
	for (uint32_t i = 0; i < animationCount; ++i) {
		model->GetAnimation(0, i, _animations[i]);
	
		
	}
	
	
	_xform = model->GetMeshXForm(0);

	
	
	_meshShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 07/Example 7.03/shaders/skinnedmesh.glsl")));
	Renderer::Texture* ptexture = _meshTexture.get();
	int texid = 0;
	_meshShader->SetTexture(texid, &ptexture, 1);
	
}


void SKINNEDMESH::Update() {
	
}

void SKINNEDMESH::SetPose(float time)
{
	_time += time*1000.f;
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
	_animatedMesh->SetAnimation(_currAnimation,false);
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
	}pushConst = { _xform*matWorld };
	
	
	_meshShader->SetUniformData("UBO", &ubo, sizeof(ubo));

	std::vector<mat4> palette;
	_animatedMesh->GetPose(palette);
	_meshShader->SetPushConstData(&pushConst, sizeof(pushConst));
	_animatedMesh->Render(_meshShader.get());
	
}