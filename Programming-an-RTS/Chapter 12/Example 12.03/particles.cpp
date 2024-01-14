#include "particles.h"

std::unique_ptr<Renderer::Texture> starTexture;
std::unique_ptr<Renderer::Shader> particleShader;

struct PARTICLE_VERTEX {
	vec4 position;
	vec4 color;	
};

constexpr int numParticles = 2048;
std::unique_ptr<Renderer::Buffer> particleBuffer;
uint32_t bufferOffset;

void LoadParticleResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager) {
	particleBuffer.reset(Renderer::Buffer::Create(pdevice, numParticles * sizeof(PARTICLE_VERTEX)));//use storage buffer instead of vertex buffer and do vertex pulling
	starTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("star.png")));
	Renderer::ShaderCreateInfo createInfo;
	createInfo.depthInfo.enable = false;
	createInfo.cullMode = Renderer::ShaderCullMode::backFace;
	createInfo.blendInfo.enable = true;
	createInfo.blendInfo.colorOp = Renderer::ShaderBlendOp::Add;
	createInfo.blendInfo.srcColorFactor = Renderer::ShaderBlendFactor::SrcAlpha;
	createInfo.blendInfo.dstColorFactor = Renderer::ShaderBlendFactor::One;
	createInfo.topologyType = Renderer::ShaderTopologyType::PointList;
	particleShader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("particles.glsl"), createInfo)));
}

void UnloadParticleResources() {
	particleBuffer.reset();
	starTexture.reset();
}

//////////////////////////////////////
// PARTIcLE SYSTEM
//////////////////////////////////////
PARTICLE_SYSTEM::PARTICLE_SYSTEM(Renderer::RenderDevice* pdevice) :EFFECT(pdevice) {
	_particleSize = 3.f;
}

PARTICLE_SYSTEM::~PARTICLE_SYSTEM() {
	_particles.clear();
}

void PARTICLE_SYSTEM::Update(float) {

}

void PARTICLE_SYSTEM::Render(mat4& matVP) {
	if (_particles.empty())
		return;
	PreRender(matVP);
	particleShader->SetTexture("texmap", &_pTexture, 1);
	
	auto buffer = particleBuffer.get();
	particleShader->SetStorageBuffer("PARTICLE_BUFFER", buffer, false);
	particleShader->Bind();
	int batchSize = 512;
	for (int i = 0; i < _particles.size(); i += batchSize)
		RenderBatch(i, batchSize);
	PostRender();
}

void PARTICLE_SYSTEM::RenderBatch(int start, int batchSize){

	//If we will reach the end of the buffer, start over
	if (bufferOffset + batchSize >= numParticles)
		bufferOffset = 0;

	PARTICLE_VERTEX* v = (PARTICLE_VERTEX*)particleBuffer->MapPtr();
	v += bufferOffset;

	int particlesRendered = 0;
	for (int i = start; i < _particles.size() && i < start + batchSize; i++) {
		if (!_particles[i].dead) {
			v->position = vec4(_particles[i].position,1.0);
			v->color = _particles[i].color;
			v++;
			particlesRendered++;
		}
	}

	particleBuffer->UnmapPtr();

	//Render batch
	if (particlesRendered > 0)
		_pdevice->DrawVertices(particlesRendered, bufferOffset);

	bufferOffset += batchSize;
}

bool PARTICLE_SYSTEM::isDead() {
	return true;
}

void PARTICLE_SYSTEM::PreRender(mat4&matVP) {
	particleShader->SetUniform("viewProj", matVP);
	particleShader->SetUniform("eyePosW", vec3(0.f, 0.f, 10.f));
	particleShader->SetUniform("sizeW", vec2(_particleSize));
}

void PARTICLE_SYSTEM::PostRender() {

}

///////////////////////////////////////////////////////////
// MAGIC_SHOWER
///////////////////////////////////////////////////////
MAGIC_SHOWER::MAGIC_SHOWER(Renderer::RenderDevice* pdevice, int noParticles, vec3 origin_) :PARTICLE_SYSTEM(pdevice), _origin(origin_) {
	///noParticles = 1;
	_particles.resize(noParticles);
	for (int i = 0; i < noParticles; i++) {
		_particles[i].time_to_live = rand() % 5000 / 1000.f;
		_particles[i].dead = true;
		_particles[i].acceleration = vec3(0.f, -0.75f, 0.f);
		_particles[i].position = vec3(0.f);
		
	}
	_pTexture = starTexture.get();
}

MAGIC_SHOWER::~MAGIC_SHOWER() {
	_particles.clear();
}

void MAGIC_SHOWER::Update(float delta) {
	for (int i = 0; i < _particles.size(); i++) {
		//Update live particles
		_particles[i].time_to_live -= delta;
		_particles[i].velocity += _particles[i].acceleration * delta;
		_particles[i].position += _particles[i].velocity * delta;
		_particles[i].color.w = _particles[i].time_to_live / 5.f;

		//respawn dead particles
		if (_particles[i].time_to_live <= 0.f) {
			_particles[i].position = _origin;

			//Random direction
			_particles[i].velocity = glm::normalize(vec3((rand() % 2000 / 1000.f) - 1.f, (rand() % 2000 / 1000.f) - 1.f, (rand() % 2000 / 1000.f) - 1.f))*2.f;

			//Random color
			_particles[i].color = vec4(rand() % 1000 / 1000.f, rand() % 1000 / 1000.f, rand() % 1000 / 1000.f,1.f);

			//Random life span
			_particles[i].time_to_live = rand() % 4000 / 1000.f + 1.f;
			_particles[i].dead = false;

		}

	}
}

bool MAGIC_SHOWER::isDead() {
	return false;
}