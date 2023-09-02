#pragma once

#include <common.h>

class SKINNEDMESH {
	Renderer::RenderDevice* _pdevice;
	std::unique_ptr<Renderer::Line2D> _line;
	std::unique_ptr<Renderer::Shader> _sphereShader;
	std::unique_ptr<Mesh::Mesh> _sphere;
	int _boneCount;
	std::vector<mat4> _boneXForms;
	std::vector<std::string> _boneNames;
	std::vector<int> _boneHierarchy;
	mat4 _xform;
public:
	SKINNEDMESH();
	~SKINNEDMESH();
	void Load(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char* fileName);
	void Update();
	void Render(mat4& matVP,mat4&matWorld,Renderer::DirectionalLight&light);
};