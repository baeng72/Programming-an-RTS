#include "skinnedMesh.h"
#include <cmath>
SKINNEDMESH::SKINNEDMESH() {
	_currAnimation = 0;
	_time = 0.f;
}

SKINNEDMESH::~SKINNEDMESH() {
	_pdevice->Wait();
	_mesh.reset();

}

void SKINNEDMESH::Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char* fileName)
{
	_pdevice = pdevice;
	std::unique_ptr<Mesh::Model> model = std::unique_ptr<Mesh::Model>(Mesh::Model::Create(pdevice, fileName));
	_meshTexture = std::unique_ptr<Renderer::Texture>(model->GetTexture(model->GetMeshMaterialIndex(0), Mesh::TextureType::diffuse, 0));
	_mesh = std::unique_ptr<Mesh::Mesh>(model->GetMesh(Mesh::MeshType::pos_norm_uv_bones, 0));
	_animatedMesh = std::unique_ptr<Mesh::AnimatedMesh>(model->GetAnimatedMesh(Mesh::MeshType::pos_norm_uv_bones, 0));
	_animationController = std::unique_ptr<Mesh::AnimationController>(_animatedMesh->GetController());
	uint32_t animationCount = model->GetAnimationCount(0);
	_animations.resize(animationCount);

	for (uint32_t i = 0; i < animationCount; ++i) {
		model->GetAnimation(0, i, _animations[i]);


	}

	_xform = model->GetMeshXForm(0);

	Renderer::ShaderStorageType shaderTypes[] = { Renderer::ShaderStorageType::Uniform,Renderer::ShaderStorageType::StorageDynamic,Renderer::ShaderStorageType::Texture };
	if (Core::GetAPI() == Core::API::Vulkan) {
		_meshShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.03/shaders/Vulkan/skinnedmesh.glsl", true, true, true,
			shaderTypes, 3)));

		/*Renderer::Texture* ptexture = _meshTexture.get();
		int texid = 0;
		_meshShader->SetTexture(texid, &ptexture, 1);*/
	}
	else {
		_meshShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.03/shaders/GL/skinnedmesh.glsl", true, true, true,
			shaderTypes, 3)));

	}

}


void SKINNEDMESH::Update() {
	_meshShader->SetStorageBuffer("skeleton", _animatedMesh->GetBoneBuffer(), true);//such a hack...
}

void SKINNEDMESH::SetPose(float time)
{

	if (_currAnimation == 2) {
		int z = 0;
	}
	_animationController->Advance(time);


}

void SKINNEDMESH::SetAnimation(const char* pname)
{
	for (size_t i = 0; i < _animations.size(); i++) {
		if (_animations[i].name == pname) {
			_currAnimation = (uint32_t)i;
			break;
		}
	}
	_animationController->SetAnimation(_currAnimation, false);
	_time = 0.f;
}

void SKINNEDMESH::SetAnimation(int animidx) {
	_currAnimation = animidx;
	_animationController->SetAnimation(_currAnimation, false);
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
void SKINNEDMESH::Render(Renderer::Shader* pshader) {

	/*{

		uint32_t dynoffsets[1] = { (uint32_t)_animationController->GetControllerOffset() * sizeof(mat4) };

		pshader->Bind(dynoffsets, 1);
	}*/
	{
		Renderer::Texture* ptexture = _meshTexture.get();
		_meshShader->SetTexture("texmap", &ptexture, 1);
		_animatedMesh->Bind();
		_animatedMesh->Render(_animationController.get());
	}

}

int SKINNEDMESH::GetBoneIndex(const char* boneName)
{
	return _animationController->GetBoneIndex(boneName);
}

mat4 SKINNEDMESH::GetBoneXForm(int boneID)
{
	mat4 xform;
	_animationController->GetBonePoseXForm(boneID, xform);
	return xform;
}
