#pragma once

#include <common.h>

class SKYBOX {
	Renderer::RenderDevice* _pdevice;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	std::unique_ptr<Mesh::MultiMesh> _mesh;
	std::unique_ptr<Renderer::Shader> _shader;
public:
	SKYBOX(Renderer::RenderDevice*pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char*pfilename,float size);
	~SKYBOX();
	void Render(mat4&viewProj,vec3 cameraPos);
};