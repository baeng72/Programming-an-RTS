#pragma once

#include <common.h>

#define ANIM_STILL 0
#define ANIM_IDLE 3
#define ANIM_RUN 2
#define ANIM_ATTACK 1
#define ANIM_DIE 4

class SKINNEDMESH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Mesh::AnimatedMesh> _animatedMesh;
	std::unique_ptr<Mesh::AnimationController> _animationController;
	std::unique_ptr<Renderer::Shader> _meshShader;
	std::unique_ptr<Renderer::Texture> _meshTexture;
	std::unique_ptr<Mesh::Mesh> _mesh;
	mat4 _xform;
	std::vector<Mesh::AnimationClip> _animations;

	uint32_t _currAnimation;
	float _time;
	float _clipLength;
public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char* fileName);
	void Update();
	void SetPose(Mesh::AnimationController*pctrl,float time);
	void SetAnimation(const char* pname);
	void SetAnimation(int animidx);
	float GetAnimationDuration(int index);
	std::vector<std::string> GetAnimations();
	void Render(Mesh::AnimationController*pcontroller,Renderer::Shader* pshader);
	
	int GetBoneIndex(const char* boneName);
	mat4 GetBoneXForm(int boneID);
	mat4 GetWorldXForm()const { return _xform; }
	float GetAnimationTime()const { return _animationController->GetTime(); }
	Mesh::AnimationController* GetAnimationController()const { return _animatedMesh->GetController(); }
	Renderer::Shader* GetShader() { return _meshShader.get(); }
	
};