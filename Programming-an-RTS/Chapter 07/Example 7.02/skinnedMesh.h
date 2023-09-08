#pragma once

#include <common.h>

class SKINNEDMESH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Renderer::Line2D> _line;
	std::unique_ptr<Renderer::Shader> _sphereShader;
	std::unique_ptr<Mesh::Mesh> _sphere;
	std::unique_ptr<Mesh::AnimatedMesh> _animatedMesh;
	int _boneCount;
	std::vector<mat4> _boneXForms;
	std::vector<std::string> _boneNames;
	std::vector<int> _boneHierarchy;
	mat4 _xform;
	std::vector<Mesh::AnimationClip> _animations;
	uint32_t _currAnimation;
	float _time;
public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char* fileName);
	void Update();
	void SetPose(float time);
	void SetAnimation(const char* pname);
	std::vector<std::string> GetAnimations();
	void Render(mat4& matVP,mat4&matWorld,Renderer::DirectionalLight&light);
};