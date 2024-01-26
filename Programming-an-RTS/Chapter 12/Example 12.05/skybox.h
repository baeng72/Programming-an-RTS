#pragma once

#include <common.h>
#include "terrain.h"
class SKYBOX {
	
	Renderer::RenderDevice* _pdevice;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	std::unique_ptr<Mesh::MultiMesh> _mesh;
	std::unique_ptr<Renderer::Shader> _shader;	
	std::unique_ptr<Renderer::Texture> _skyboxTexture;
	std::unique_ptr<Renderer::Texture> _depthTexture;
	std::unique_ptr<Renderer::FrameBuffer> _skyboxFramebuffer;
	//std::unique_ptr<Renderer::Shader> _fowShader;
public:
	SKYBOX(Renderer::RenderDevice*pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, const char*pfilename,float size);
	~SKYBOX();
	void Render(mat4&viewProj,vec3 cameraPos);
	void GenerateEnvironmentMap(vec3 position, bool saveToFile,TERRAIN& _terrain,Renderer::DirectionalLight& light,Core::Window*pwindow);
};