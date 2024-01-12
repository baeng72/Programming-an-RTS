#include "effect.h"

//Global effect variables
std::unique_ptr<Mesh::Mesh> billboardMesh;
std::unique_ptr<Renderer::Shader> effectShader;

//Global effect textures
std::unique_ptr<Renderer::Texture> runesTexture;
std::unique_ptr<Renderer::Texture> cloudTexture;


struct SimpleVertex {
	vec3 position;
	vec3 normal;
	vec2 uv;
	SimpleVertex() {}
	SimpleVertex(vec3 pos_, vec3 norm_, vec2 uv_) {
		position = pos_;
		normal = norm_;
		uv = uv_;
	}
};

void LoadEffectResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager) {
	//Calculate sight mesh (a simple quad)
	
	//Create 4 vertices
	std::vector<SimpleVertex> v(4);
	v[0] = SimpleVertex(vec3(-0.5f, 0.f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 0.f));
	v[1] = SimpleVertex(vec3(0.5f, 0.f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 0.f));
	v[2] = SimpleVertex(vec3(0.5f, 0.f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 1.f));
	v[3] = SimpleVertex(vec3(-0.5f, 0.f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 1.f));

	//Create 2 faces
	std::vector<uint32_t> indices(6);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	Renderer::VertexAttributes attributes = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2},sizeof(SimpleVertex) };

	billboardMesh.reset(Mesh::Mesh::Create(pdevice, (float*)v.data(),(uint32_t) (sizeof(SimpleVertex) * v.size()), indices.data(), (uint32_t) (sizeof(uint32_t) * indices.size()), attributes));
	Renderer::ShaderCreateInfo createinfo;
	createinfo.cullMode = Renderer::ShaderCullMode::None;
	createinfo.depthInfo.enable = false;
	createinfo.blendInfo.enable = true;
	createinfo.blendInfo.srcColorFactor = Renderer::ShaderBlendFactor::SrcAlpha;
	createinfo.blendInfo.dstColorFactor = Renderer::ShaderBlendFactor::One;
	createinfo.blendInfo.colorOp = Renderer::ShaderBlendOp::Add;
	createinfo.blendInfo.srcAlphaFactor = Renderer::ShaderBlendFactor::SrcAlpha;
	createinfo.blendInfo.dstAlphaFactor = Renderer::ShaderBlendFactor::DstAlpha;
	createinfo.blendInfo.alphaOp = Renderer::ShaderBlendOp::Add;
	effectShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("effect.glsl"),createinfo)));

	runesTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("runes.png")));
	cloudTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("cloud.png")));

}

void UnloadEffectResources() {
	billboardMesh.reset();
	effectShader.reset();
	runesTexture.reset();
	cloudTexture.reset();
}
//////////////////////////////////////////////
// TRANSFORM
//////////////////////////////////////////////

TRANSFORM::TRANSFORM() {
	_pos = _rot = vec3(0.f);
	_sca = vec3(1.f);
}

TRANSFORM::TRANSFORM(vec3 pos) {
	_pos = pos;
	_rot = vec3(0.f);
	_sca = vec3(1.f);
}

TRANSFORM::TRANSFORM(vec3 pos, vec3 rot) {
	_pos = pos;
	_rot = rot;
	_sca = vec3(1.f);
}

TRANSFORM::TRANSFORM(vec3 pos, vec3 rot, vec3 sca) {
	_pos = pos;
	_rot = rot;
	_sca = sca;
}

void TRANSFORM::Init(vec3 pos) {
	_pos = pos;
	_rot = vec3(0.f);
	_sca = vec3(1.f);
}

void TRANSFORM::Init(vec3 pos, vec3 rot) {
	_pos = pos;
	_rot = rot;
	_sca = vec3(1.f);
}

void TRANSFORM::Init(vec3 pos, vec3 rot, vec3 sca) {
	_pos = pos;
	_rot = rot;
	_sca = sca;
}

mat4 TRANSFORM::GetWorldMatrix() {
	mat4 id = mat4(1.f);
	mat4 p = translate(id, _pos);
	mat4 rx = rotate(id, _rot.x,vec3(1.f,0.f,0.f));
	mat4 ry = rotate(id, _rot.y, vec3(0.f, 1.f, 0.f));
	mat4 rz = rotate(id, _rot.z, vec3(0.f, 0.f, 1.f));
	mat4 s = scale(id, _sca);
	mat4 world = p * rz * ry * rx * s;
	return world;
}

///////////////////////
/// EFFECT BASE CLASS
///////////////////////
EFFECT::EFFECT(Renderer::RenderDevice* pdevice) :_pdevice(pdevice) {
	_time = 0.f;
	_color = Color(1.f);//white
}

void EFFECT::PreRender(mat4&matVP) {
	effectShader->Bind();
	effectShader->SetUniform("matVP", matVP);
	effectShader->SetUniform("color", _color);
	
}

void EFFECT::PostRender() {

}

////////////////////////////////////
// EFFECT SPELL
////////////////////////////////////
EFFECT_SPELL::EFFECT_SPELL(Renderer::RenderDevice* pdevice, vec3 pos) :EFFECT(pdevice) {
	_t1._pos = pos;
	_t1._rot = vec3(0.f);
	_t1._sca = vec3(0.1f);
	_t1._pos.y += 1.f;

	//Set a random color of the effect
	_color = Color(rand() % 1000 / 1000.f, rand() % 1000 / 1000.f, rand() % 1000 / 1000.f, 0.f);

	//Initiate the glow transforms
	for (int i = 0; i < 10; i++) {
		float angle = i * (glm::pi<float>() / 5.f);
		_c[i].Init(pos + vec3(cos(angle) * 0.5f, 2.5f, sin(angle) * 0.5f), vec3(glm::pi<float>() * 0.5f, angle, 0.f), vec3(4.f, 4.f, 6.f));
	}
}

void EFFECT_SPELL::Update(float delta) {
	_time += delta;

	//Update lower spinning quad
	_t1._rot.y += delta;

	//update cloud
	for (int i = 0; i < 10; i++) {
		_c[i]._rot.y -= delta;
	}

	//Update spinning quad scale
	if (_time < 1.5f)
		_t1._sca += vec3(delta) * 4.f;
	else if (_time > 4.5f)
		_t1._sca -= vec3(delta) * 4.f;

	//Calculate alpha
	_color.w = _t1._sca.x / 6.f;
}

bool EFFECT_SPELL::isDead() {
	return _t1._sca.x < 0.f;
}

void EFFECT_SPELL::Render(mat4&matVP) {
	PreRender(matVP);
	if (billboardMesh) {
		//Spinning quad
		auto texture = runesTexture.get();
		effectShader->SetTexture("effectTexture", &texture,1);
		effectShader->SetUniform("matWorld", _t1.GetWorldMatrix());
		effectShader->Bind();
		billboardMesh->Bind();
		billboardMesh->Render();
		//Glow
		texture = cloudTexture.get();
		effectShader->SetTexture("effectTexture", &texture, 1);
		effectShader->Bind();
		for (int i = 0; i < 10; i++) {
			effectShader->SetUniform("matWorld", _c[i].GetWorldMatrix());
			billboardMesh->Render();
		}

	}
}


