#include "effect.h"

//Global effect variables
std::unique_ptr<Mesh::Mesh> billboardMesh;
std::unique_ptr<Renderer::Shader> effectShader;
std::unique_ptr<Renderer::Sprite> sprite;

//Global effect textures
std::unique_ptr<Renderer::Texture> runesTexture;
std::unique_ptr<Renderer::Texture> cloudTexture;
std::unique_ptr<Renderer::Texture> fireballTexture;
std::unique_ptr<Renderer::Texture> lensFlareTexture;

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

	sprite.reset(Renderer::Sprite::Create(pdevice));

	runesTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("runes.png")));
	cloudTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("cloud.png")));
	fireballTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("fireball.png")));
	lensFlareTexture.reset(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("lensflare.png")));
}

void UnloadEffectResources() {
	billboardMesh.reset();
	effectShader.reset();
	sprite.reset();
	lensFlareTexture.reset();
	fireballTexture.reset();
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


/////////////////////////////////////
// EFFECT_FIREBALL
/////////////////////////////////////
EFFECT_FIREBALL::EFFECT_FIREBALL(Renderer::RenderDevice* pdevice,mat4&loc, SKINNEDMESH&skinnedMesh,int boneID, vec3 dest) :EFFECT(pdevice),_loc(loc), _skinnedMesh(skinnedMesh),_boneID(boneID), _dest(dest) {
	_color.w = 0.01f;
	_prc = 0;
	_speed = 22.f;
	mat4 src = skinnedMesh.GetBoneXForm(boneID);
	_t1.Init(src[3], vec3(0.f), vec3(0.1f));
}

void EFFECT_FIREBALL::Update(float delta) {
	_t1._rot += vec3(0.5f) * delta;
	if (_time < 1.f) {
		//follow staff (Phase 1)
		_time += delta;
		
		mat4 boneXForm = _skinnedMesh.GetBoneXForm(_boneID);
		mat4 loc = boneXForm*_loc;
		vec3 hack = vec3(1.f, -2.f, 11.f);//hack, pure and simple. We don't load bones that have no animation channels, i.e. the bone 'Staff', so need to align with a nearby bone that exists
		_t1._pos = vec3(loc[3])-hack;
		_t1._sca = vec3(1.5f) * _time;

		//Fade in fireball (w = alpha)
		_color.w = _time;

		if (_time > 1.f) {
			_color.w = _time * 0.5f;
			_origin = _t1._pos;
			vec3 vec = (_origin - _dest);
			_length = glm::length(vec);
		}
	}
	else if (_prc < 1.f) {
		//Fly towards target (Phase 2)
		_prc += (_speed * delta) / _length;
		_t1._pos = GetPosition(_prc);
	}
	else {
		//Explode (Phase 3)
		_prc += (_speed * delta) / _length;
		_t1._sca += vec3(5.f) * delta;
		_color.w -= delta * 0.25f;
	}
}

void EFFECT_FIREBALL::Render(mat4& matVP) {
	PreRender(matVP);
	if (billboardMesh) {
		//Create a Trail of small fireballs
		vec3 orgRot = _t1._rot;
		vec3 rotations[] = { vec3(0.f),
			vec3(pi * 0.5f,0.f,0.f),
			vec3(0.f, pi,0.f),
			vec3(0.f,0.f,pi * 0.5f)
		};
		vec3 orgPos = _t1._pos;
		vec3 positions[] = {
			_t1._pos,
			GetPosition(_prc - (1.5f / _length)),
			GetPosition(_prc - (2.5f / _length)),
			GetPosition(_prc - (3.25f / _length)),
			GetPosition(_prc - (4.f / _length))
		};

		vec3 orgSca = _t1._sca;
		vec3 scales[] = {
			_t1._sca,
			_t1._sca * 0.8f,
			_t1._sca * 0.6f,
			_t1._sca * 0.4f,
			_t1._sca * 0.2f
		};

		auto texture = fireballTexture.get();
		effectShader->SetTexture("effectTexture", &texture, 1);
		billboardMesh->Bind();
		for (int t = 0; t < 5; t++) {
			for (int i = 0; i < 4; i++) {
				_t1._pos = positions[t];
				_t1._rot = orgRot + rotations[i];
				_t1._sca = scales[t];
				effectShader->SetUniform("matWorld", _t1.GetWorldMatrix());
				billboardMesh->Render();
			}
		}
		_t1._pos = orgPos;
		_t1._rot = orgRot;
		_t1._sca = orgSca;

	}
	PostRender();
}


bool EFFECT_FIREBALL::isDead() {
	return _color.w < 0.f;
}

vec3 EFFECT_FIREBALL::GetPosition(float p) {
	if (p < 0.f)
		p = 0.f;
	if (p > 1.f)
		p = 1.f;
	vec3 pos = _origin * (1.f - p) + _dest * p;	//lerp between origin and dest
	pos.y += std::sin(p * pi) * 3.f;	//add arc
	return pos;
}

/////////////////////////////////////
// EFFECT_LENSFLARE
/////////////////////////////////////

EFFECT_LENSFLARE::EFFECT_LENSFLARE(Renderer::RenderDevice* pdevice, int type, vec3 position)
	:EFFECT(pdevice)
{
	_position = position;
	_type = type;
	_mainAlpha = 0.f;
	_inScreen = false;

	//Add Flares
	if (_type == 0) {
		//Standare flare
		_flares.push_back(FLARE(Color(1.0f, 1.0f, 0.5f, 1.0f), 0.5f, 0.7f, 0));
		_flares.push_back(FLARE(Color(0.0f, 1.0f, 0.5f, 1.0f), 1.0f, 1.0f, 1));
		_flares.push_back(FLARE(Color(1.0f, 0.5f, 0.5f, 1.0f), 1.5f, 1.3f, 2));
		_flares.push_back(FLARE(Color(1.0f, 1.0f, 0.5f, 1.0f), -0.5f, 0.8f, 3));
		_flares.push_back(FLARE(Color(0.0f, 1.0f, 0.5f, 1.0f), 0.4f, 1.0f, 4));
		_flares.push_back(FLARE(Color(1.0f, 1.0f, 0.5f, 1.0f), 0.75f, 1.0f, 5));
		_flares.push_back(FLARE(Color(0.0f, 0.0f, 1.0f, 1.0f), 1.8f, 1.2f, 6));
		_flares.push_back(FLARE(Color(1.0f, 1.0f, 0.5f, 1.0f), 2.1f, 0.5f, 4));
	}
	else if (_type == 1) {
		//another flare 
	}
	int width, height;
	_pdevice->GetDimensions(&width, &height);
	_viewport = { 0.f,0.f,(float)width,(float)height };
}

void EFFECT_LENSFLARE::Update(float delta)
{
	if (_inScreen)
		_mainAlpha += delta * 3.f;
	else
		_mainAlpha -= delta * 3.f;

	if (_mainAlpha > 1.f)
		_mainAlpha = 1.f;
	if (_mainAlpha < 0.f)
		_mainAlpha = 0.f;
}

void EFFECT_LENSFLARE::Render(mat4& matVP)
{
	Rect sourceRectangles[7] = { {0, 0, 128, 128},
								{128, 0, 256, 128},
								{0, 128, 128, 256},
								{128, 128, 192, 192},
								{192, 128, 256, 192},
								{128, 192, 192, 256},
								{192, 192, 256, 256} };
	
	vec3 screenPos = Core::project(_position, mat4(1.f), matVP, _viewport);

	//check that light source is within or without the screen bound
	_inScreen = true;
	if (screenPos.x < 0 || screenPos.x > _viewport.z ||
		screenPos.y < 0 || screenPos.y > _viewport.w || screenPos.z > 1.f)
		_inScreen = false;

	//LensFlares aren't visible if Alpha == 0 ,so exit
	if (_mainAlpha <= 0.f)
		return;

	//Calculate ray from screen centre to the light source
	vec2 lightSource = vec2(screenPos.x, screenPos.y);
	vec2 screenCentre = vec2(_viewport.z * 0.5f, _viewport.w * 0.5f);
	vec2 ray = screenCentre - lightSource;

	//Draw the different flares
	mat4 sca;
	auto texture = lensFlareTexture.get();
	/*effectShader->SetTexture("effectTexture", &texture, 1);
	effectShader->SetUniform("matVP", matVP);
	effectShader->Bind();*/
	//billboardMesh->Bind();
	for (int i = 0; i < _flares.size(); i++) {
		//calculate flare position in screen coordinates
		Rect r = sourceRectangles[_flares[i]._sourceFlare];
		vec2 offset = vec2(r.Width() * 0.5f, r.Height() * 0.5f) * _flares[i]._scale;
		vec2 flarePos = lightSource + ray * _flares[i]._place - offset;

		//scale
		sca = glm::scale(mat4(1.f), vec3(_flares[i]._scale, _flares[i]._scale, 1.f));

		//Calcualte flare alpha
		Color color = _flares[i]._color;
		vec2 v2 = ((flarePos + offset) - screenCentre);
		float alpha = (length(v2) + 150.f) / (_viewport.w - _viewport.y);
		if (alpha > 1.f)
			alpha = 1.f;
		color.a = alpha * _mainAlpha * 1.5f;
		
		//effectShader->SetUniform("color", color);
		
		//Draw Flare
		vec3 vec = vec3(flarePos.x / _flares[i]._scale, flarePos.y / _flares[i]._scale, 0.f);
		//effectShader->SetUniform("model", sca);
		//billboardMesh->Render();
		sprite->SetTransform(sca);
		sprite->Draw(texture,r, vec,color);
	}
}

bool EFFECT_LENSFLARE::isDead()
{
	return false;
}
