#pragma once

#include <common.h>

class SKINNEDMESH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Mesh::AnimatedMesh> _animatedMesh;
	std::unique_ptr<Renderer::Shader> _meshShader;
	std::unique_ptr<Renderer::Texture> _meshTexture;
	std::unique_ptr<Mesh::Mesh> _mesh;
	mat4 _xform;
	std::vector<Mesh::AnimationClip> _animations;
	
	uint32_t _currAnimation;
	
	float _clipLength;
public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char* fileName);
	void Update();
	void SetPose(float time,Mesh::AnimationController*pcontroller);
	void SetAnimation(const char* pname, Mesh::AnimationController* pcontroller);
	void SetAnimation(int i, Mesh::AnimationController* pcontroller);
	std::vector<std::string> GetAnimations();
	void Render(mat4& matVP,mat4&matWorld,Renderer::DirectionalLight&light,vec4&color, Mesh::AnimationController* pcontroller);
	int GetBoneIndex(const char* boneName, Mesh::AnimationController* pcontroller);
	mat4 GetBoneXForm(int boneID, Mesh::AnimationController* pcontroller);
	Mesh::AnimationController* GetAnimationController();
};