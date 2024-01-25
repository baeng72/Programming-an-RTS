#include "mouse.h"

MOUSE::MOUSE() {
	_type = 0;
	_speed = 1.5f;
	_disableTime = 0.f;
	yscroll = 0.f;
}

MOUSE::~MOUSE() {
	_pwindow->ShowCursor(true);
	_textures.clear();
}

void MOUSE::Init(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, Core::Window* pwindow) {
	_pdevice = pdevice;
	_pwindow = pwindow;
	int width, height;
	_pwindow->GetWindowSize(width, height);
	x = width / 2;
	y = height / 2;
	pwindow->ShowCursor(false);
	std::unique_ptr<Renderer::Image> image = std::unique_ptr<Renderer::Image>(Renderer::Image::Create("../../../../Resources/Chapter 12/Example 12.04/cursor/cursor.png"));
	uint32_t* pixels = (uint32_t*)image->GetPixels();
	int channels;
	image->GetSize(width, height, channels);
	//create cursors
	Rect rects[] = {
		{0,0,20,20},
		{0,20,20,40},
		{20,20,40,40},
		{0,40,20,60},
		{20,40,40,60}
	};
	uint32_t* newpixels = new uint32_t[20 * 20];
	_textures.resize(5);
	for (int i = 0; i < 5; i++) {
		assert(rects[i].right - rects[i].left == 20);
		assert(rects[i].bottom - rects[i].top == 20);
		for (int y = rects[i].top, y0 = 0; y < rects[i].bottom; y++, y0++) {
			for (int x = rects[i].left, x0 = 0; x < rects[i].right; x++, x0++) {
				newpixels[x0 + y0 * 20] = pixels[x + y * width];
			}
		}
		_textures[i] = std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, 20, 20, Renderer::TextureFormat::R8G8B8A8, (uint8_t*)newpixels));
	}
	delete[] newpixels;

	_sprite = std::unique_ptr<Renderer::Sprite>(Renderer::Sprite::Create(pdevice));

	pwindow->GetWindowSize(width, height);
	_viewport = { 0,0,width,height };
	x = (int)(_viewport.left + _viewport.Width() / 2.f);
	y = (int)(_viewport.top + _viewport.Height() / 2.f);


	std::unique_ptr<Mesh::Shape> shape;
	shape.reset(Mesh::Shape::Create(pdevice));
	_sphereMesh = std::unique_ptr<Mesh::Mesh>(shape->CreateSphere(0.2f, 5, 5));

	_sphereShader = std::unique_ptr<Renderer::Shader>(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("shape.glsl"), Renderer::ShaderCullMode::frontFace)));
}



void MOUSE::Paint(mat4&matVP, Renderer::DirectionalLight& light) {

	mat4 world = glm::translate(mat4(1.f), _ballPos);
	auto color = Color(0.8f, 0.8f, 0.8f, 1.0f);
	
	_sphereShader->Bind();
	_sphereShader->SetUniform("viewProj", &matVP);
	_sphereShader->SetUniform("model", &world);
	_sphereShader->SetUniform("color", &color);
	_sphereShader->SetUniform("light.ambient", &light.ambient);
	_sphereShader->SetUniform("light.diffuse", &light.diffuse);
	_sphereShader->SetUniform("light.specular", &light.specular);
	_sphereShader->SetUniform("light.direction", &light.direction);
	
	_sphereMesh->Bind();
	_sphereMesh->Render();
	_sprite->Draw(_textures[_type].get(), vec3(x, y, 0.f));
}

RAY MOUSE::GetRay(mat4& matProj, mat4& matView, mat4& matWorld)
{
	float width = (float)(_viewport.right - _viewport.left);
	float height = (float)(_viewport.bottom - _viewport.top);
	float a = matProj[0][0];
	float b = matProj[1][1];
	int32_t tx = x;
	int32_t ty = y;
	//convert from screen coordinates to ndc
	float angle_x;
	float angle_y;
	
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__
	angle_x = (((2.f * tx) / width) - 1.f) / a;
	angle_y = (((2.f * ty) / height) - 1.f) / b;
#else
	if (Core::GetAPI() == Core::API::Vulkan) {
		angle_x = (((2.f * tx) / width) - 1.f) / a;
		angle_y = (((2.f * ty) / height) - 1.f) / b;
	}
	else {
		angle_x = (((2.f * tx) / width) - 1.f) / a;
		angle_y = (((-2.f * ty) / height) + 1.f) / b;
	}
#endif

	vec4 org = vec4(0.f, 0.f, 0.f, 1.f);
	vec4 dir = vec4(angle_x, angle_y, 1.f, 0.f);

	//review view world xform
	mat4 vw = matView * matWorld;
	mat4 invVW = glm::inverse(vw);

	RAY ray;
	ray.org = invVW * org;
	ray.dir = glm::normalize(invVW * dir);

	return ray;
}


void MOUSE::DisableInput(float ms)
{
	
	_disableTime = ms+ _pdevice->GetCurrentTicks();
}


bool MOUSE::Over(Rect& dst) {
	if (x < dst.left || x > dst.right)
		return false;
	if (y < dst.top || y > dst.bottom)
		return false;
	return true;
}

bool MOUSE::PressInRect(Rect& dst) {
	return (ClickLeft() && Over(dst));
}

bool MOUSE::ClickLeft() {
	return _pwindow->IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool MOUSE::ClickRight() {
	return _pwindow->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

bool MOUSE::WheelDown() {
	return yscroll < 0.f;
}

bool MOUSE::WheelUp() {
	return yscroll > 0.f;
}

/// <summary>
/// RAY
/// </summary>

RAY::RAY()
{
	org = dir = vec3(0.f);
}

RAY::RAY(vec3 o, vec3 d)
{
	org = o;
	dir = d;
}



float RAY::Intersect(std::vector<vec3>& vertices, std::vector<uint32_t>& indices) {
	bool hit = false;
	float dist = INFINITY;
	uint32_t hitTri = UINT32_MAX;
	uint32_t currTri = 0;
	for (size_t f = 0; f < indices.size(); f += 3) {
		uint32_t i0 = indices[f + 0];
		uint32_t i1 = indices[f + 1];
		uint32_t i2 = indices[f + 2];
		vec3 v0 = vertices[i0];
		vec3 v1 = vertices[i1];
		vec3 v2 = vertices[i2];
		vec2 bary;
		float currDist = 0.f;

		if (glm::intersectRayTriangle(org, dir, v0, v1, v2, bary, currDist)) {
			//if (currDist < dist) {
				hit = true;
				hitTri = currTri;
				dist = std::min(dist, currDist);
			//}
		}
		currTri++;
	}
	return hit ? dist : -1.f;
}

float RAY::Intersect(std::vector<vec3>& vertices, std::vector<uint32_t>& indices, uint32_t& face, vec2& bary) {
	bool hit = false;
	float dist = INFINITY;
	uint32_t hitTri = UINT32_MAX;
	uint32_t currTri = 0;
	for (size_t f = 0; f < indices.size(); f += 3) {
		uint32_t i0 = indices[f + 0];
		uint32_t i1 = indices[f + 1];
		uint32_t i2 = indices[f + 2];
		vec3 v0 = vertices[i0];
		vec3 v1 = vertices[i1];
		vec3 v2 = vertices[i2];
		
		float currDist = 0.f;

		if (glm::intersectRayTriangle(org, dir, v0, v1, v2, bary, currDist)) {
			//if (currDist < dist) {
			hit = true;
			face = hitTri = currTri;
			dist = std::min(dist, currDist);
			//}
		}
		currTri++;
	}
	return hit ? dist : -1.f;
}

void MOUSE::Update() {
	if (_pdevice->GetCurrentTicks() * 1000 < _disableTime) {
		return;
	}
	float xpos, ypos;
	_pwindow->GetCursorPos(xpos, ypos);

	x = (int)(xpos * _speed);
	y = (int)(ypos * _speed);

	_pwindow->GetScrollPos(xpos, ypos);
	yscroll = ypos;

	//Keep mouse pointer within window
	if (x < _viewport.left)
		x = _viewport.left;
	if (x < _viewport.top)
		x = _viewport.top;
	if (y > _viewport.right)
		y = _viewport.right;
	if (y > _viewport.bottom)
		y = _viewport.bottom;


}