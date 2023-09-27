#pragma once

#include <common.h>

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
	void Load(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager, const char* fileName);
	void Update();
	void SetPose(float time);
	void SetAnimation(const char* pname);
	void SetAnimation(int animidx);
	std::vector<std::string> GetAnimations();
	void Render(Renderer::Shader*pshader);
	int GetBoneIndex(const char* boneName);
	mat4 GetBoneXForm(int boneID);
	mat4 GetWorldXForm()const { return _xform; }
	Renderer::Shader* GetShader() { return _meshShader.get(); }
};