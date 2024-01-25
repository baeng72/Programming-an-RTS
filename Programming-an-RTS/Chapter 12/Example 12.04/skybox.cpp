#include "skybox.h"

struct SKYBOX_VERTEX {
	vec3 pos;
	vec2 uv;
	SKYBOX_VERTEX() {
		pos = vec3(0.f);
		uv = vec2(0.f);
	}
	SKYBOX_VERTEX(vec3 p, vec2 tx) {
		pos = p;
		uv = tx;
	}
};

SKYBOX::SKYBOX(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager, const char* pfilename, float size)
	:_pdevice(pdevice)
{
	std::string endings[] = { "_UP.jpg","_FR.jpg","_BK.jpg","_RT.jpg","_LF.jpg","_DN.jpg" };
	//Load the 6 skybox textures
	_textures.resize(6);
	for (int i = 0; i < 6; i++) {
		std::string fname = pfilename + endings[i];
		_textures[i].reset(Renderer::Texture::Create(pdevice, fname.c_str()));
	}
	std::vector<SKYBOX_VERTEX> v(24);
	vec3 corners[8] = { vec3(-size,  size,  size),
								vec3(size,  size,  size),
								vec3(-size,  size, -size),
								vec3(size,  size, -size),
								vec3(-size, -size,  size),
								vec3(size, -size,  size),
								vec3(-size, -size, -size),
								vec3(size, -size, -size) };
	std::vector<Mesh::Primitive> primitives(6);
	primitives[0].vertexStart = 0;
	primitives[0].vertexCount = 4;
	
	//Up Face
	v[0] = SKYBOX_VERTEX(corners[1], vec2(0.0f, 0.0f));
	v[1] = SKYBOX_VERTEX(corners[0], vec2(1.0f, 0.0f));
	v[2] = SKYBOX_VERTEX(corners[3], vec2(0.0f, 1.0f));
	v[3] = SKYBOX_VERTEX(corners[2], vec2(1.0f, 1.0f));

	
	primitives[1].vertexStart = 4;
	primitives[1].vertexCount = 4;
	//Front Face
	v[4] = SKYBOX_VERTEX(corners[0], vec2(0.0f, 0.0f));
	v[5] = SKYBOX_VERTEX(corners[1], vec2(1.0f, 0.0f));
	v[6] = SKYBOX_VERTEX(corners[4], vec2(0.0f, 1.0f));
	v[7] = SKYBOX_VERTEX(corners[5], vec2(1.0f, 1.0f));

	primitives[2].vertexStart = 8;
	primitives[2].vertexCount = 4;
	//Back Face
	v[8] = SKYBOX_VERTEX(corners[3], vec2(0.0f, 0.0f));
	v[9] = SKYBOX_VERTEX(corners[2], vec2(1.0f, 0.0f));
	v[10] = SKYBOX_VERTEX(corners[7], vec2(0.0f, 1.0f));
	v[11] = SKYBOX_VERTEX(corners[6], vec2(1.0f, 1.0f));

	primitives[3].vertexStart = 12;
	primitives[3].vertexCount = 4;
	//Right Face
	v[12] = SKYBOX_VERTEX(corners[2], vec2(0.0f, 0.0f));
	v[13] = SKYBOX_VERTEX(corners[0], vec2(1.0f, 0.0f));
	v[14] = SKYBOX_VERTEX(corners[6], vec2(0.0f, 1.0f));
	v[15] = SKYBOX_VERTEX(corners[4], vec2(1.0f, 1.0f));

	primitives[4].vertexStart = 16;
	primitives[4].vertexCount = 4;
	//Left Face
	v[16] = SKYBOX_VERTEX(corners[1], vec2(0.0f, 0.0f));
	v[17] = SKYBOX_VERTEX(corners[3], vec2(1.0f, 0.0f));
	v[18] = SKYBOX_VERTEX(corners[5], vec2(0.0f, 1.0f));
	v[19] = SKYBOX_VERTEX(corners[7], vec2(1.0f, 1.0f));

	primitives[5].vertexStart = 20;
	primitives[5].vertexCount = 4;
	//Down Face
	v[20] = SKYBOX_VERTEX(corners[7], vec2(0.0f, 0.0f));
	v[21] = SKYBOX_VERTEX(corners[6], vec2(1.0f, 0.0f));
	v[22] = SKYBOX_VERTEX(corners[5], vec2(0.0f, 1.0f));
	v[23] = SKYBOX_VERTEX(corners[4], vec2(1.0f, 1.0f));

	std::vector<uint32_t> ind(36);
	int index = 0;
	for (int quad = 0; quad < 6; quad++)
	{
		primitives[quad].indexCount = 6;
		primitives[quad].indexStart = quad*6;
		primitives[quad].materialIndex = 0;
		////First face
		//ind[index++] = quad * 4;
		//ind[index++] = quad * 4 + 1;
		//ind[index++] = quad * 4 + 2;

		////Second Face
		//ind[index++] = quad * 4 + 1;
		//ind[index++] = quad * 4 + 3;
		//ind[index++] = quad * 4 + 2;
		ind[index++] = 0;
		ind[index++] = 1;
		ind[index++] = 2;
		ind[index++] = 1;
		ind[index++] = 3;
		ind[index++] = 2;

		
	}
	uint32_t vertSize = sizeof(SKYBOX_VERTEX) * (uint32_t)v.size();
	uint32_t indSize = sizeof(uint32_t) * (uint32_t)ind.size();
	std::vector<float> ver(v.size()*5);//super hack, need a constructor that takes (float*)
	memcpy(ver.data(), v.data(), vertSize);
	Renderer::VertexAttributes attrs = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2},sizeof(SKYBOX_VERTEX) };
	_mesh.reset(Mesh::MultiMesh::Create(pdevice, mat4(1.f),ver, ind, primitives, attrs));
	Renderer::ShaderCreateInfo ci;
	ci.cullMode = Renderer::ShaderCullMode::frontFace;
	_shader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("skybox.glsl"),ci)));
}

SKYBOX::~SKYBOX()
{
	for (auto& tex : _textures)
		tex.reset();
	_textures.clear();
	_mesh.reset();
}

void SKYBOX::Render(mat4&viewProj,vec3 cameraPos)
{
	mat4 position = translate(mat4(1.f), cameraPos);
	_shader->SetUniform("viewProj", viewProj);
	_shader->SetUniform("world", position);
	_mesh->Bind();
	for (int i = 0; i < 6; i++) {
		auto texture = _textures[i].get();
		_shader->SetTexture("texmap", &texture, 1);
		_shader->Bind();
		_mesh->Render(i);
	}
}
