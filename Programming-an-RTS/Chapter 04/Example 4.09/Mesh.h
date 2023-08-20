#pragma once
#include <common.h>

class MESH {
	Renderer::RenderDevice* _pdevice;
	std::shared_ptr<Renderer::ShaderManager> _shaderManager;
	std::unique_ptr<Renderer::Mesh> _mesh;
	std::unique_ptr<Renderer::Texture> _texture;
	
	std::unique_ptr<Renderer::Shader> _shader;
	glm::mat4 _xform;
	void LoadShader();
public:
	MESH();
	MESH( Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> pshadermanager, const char* pName);
	~MESH();
	bool Load( Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager> shadermanager, const char* pName );
	void Render(glm::mat4& matViewProj, glm::mat4& matWorld,Renderer::DirectionalLight&light);
	void Release();
};