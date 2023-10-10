#include "mouse.h"

MOUSE::MOUSE() {
	_type = 0;
	_speed = 1.5f;
	

}

MOUSE::~MOUSE() {
	_pwindow->ShowCursor(true);
	_textures.clear();
}

void MOUSE::Init(Renderer::RenderDevice* pdevice, Core::Window*pwindow) {
	_pdevice = pdevice;
	_pwindow = pwindow;
	int width, height;
	_pwindow->GetWindowSize(width, height);
	x = width / 2;
	y = height / 2;
	pwindow->ShowCursor(false);
	std::unique_ptr<Renderer::Image> image = std::unique_ptr<Renderer::Image>(Renderer::Image::Create("../../../../Resources/Chapter 05/Example 5.03/cursors/cursor.png"));
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
		for (int y = rects[i].top,y0=0; y < rects[i].bottom; y++,y0++) {
			for (int x = rects[i].left,x0=0; x < rects[i].right; x++,x0++) {
				newpixels[x0 + y0 * 20] = pixels[x + y * width];
			}
		}
		_textures[i] = std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, 20, 20, 4,(uint8_t*) newpixels));
	}
	delete[] newpixels;

	_sprite = std::unique_ptr<Renderer::Sprite>(Renderer::Sprite::Create(pdevice));

	pwindow->GetWindowSize(width, height);
	_viewport = { 0,0,width,height };
}

void MOUSE::Update() {
	float xpos, ypos;
	_pwindow->GetCursorPos(xpos, ypos);
	
	x = (int)(xpos*_speed);
	y = (int)(ypos*_speed);

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

void MOUSE::Paint() {	
	_sprite->Draw(_textures[_type].get(), glm::vec3(x, y, 0.f));
}

RAY MOUSE::GetRay(glm::mat4& matProj, glm::mat4& matView, glm::mat4& matWorld)
{
	float width = (float)(_viewport.right - _viewport.left);
	float height = (float)(_viewport.bottom - _viewport.top);
	float a = matProj[0][0];
	float b = matProj[1][1];
	int32_t tx = x;
	int32_t ty = y;
	//convert from screen coordinates to ndc
	float angle_x;
	float angle_y;//viewport flipped in vulkan
	//float angle_y = (((-2.f * ty) / height) + 1.f) / b;
	if (Core::GetAPI() == Core::API::Vulkan) {
		angle_x = (((2.f * tx) / width) - 1.f) / a;
		angle_y = (((2.f * ty) / height) - 1.f) / b;//viewport flipped in vulkan
	}
	else {
		angle_x = (((2.f * tx) / width) - 1.f) / a;
		angle_y = (((-2.f * ty) / height) + 1.f) / b;
	}
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

float RAY::Intersect(MESHINSTANCE iMesh)
{
	if (iMesh._mesh == nullptr)
		return -1;
	auto& vertices = iMesh._mesh->_vertices;
	auto& indices = iMesh._mesh->_indices;
	bool hit = false;
	float dist = INFINITY;
	uint32_t hitTri = UINT32_MAX;
	uint32_t currTri = 0;
	for (size_t f = 0; f < indices.size(); f += 3) {
		uint32_t i0 = indices[f + 0];
		uint32_t i1 = indices[f + 1];
		uint32_t i2 = indices[f + 2];
		vec3 v0 = vec3(iMesh._mesh->_xform*vec4(vertices[i0],1.f));
		vec3 v1 = vec3(iMesh._mesh->_xform*vec4(vertices[i1],1.f));
		vec3 v2 = vec3(iMesh._mesh->_xform*vec4(vertices[i2],1.f));
		vec2 bary;
		float currDist = 0.f;
		
		if (glm::intersectRayTriangle(org, dir, v0, v1, v2, bary, currDist)) {
			if (currDist < dist) {
				hit = true;
				hitTri = currTri;
				dist = currDist;
			}
		}
		currTri++;
	}
	return hit ? dist : -1.f;
}

float RAY::Intersect(BBOX bBox)
{
	vec3 min = bBox.min;
	vec3 max = bBox.max;
	float dist = -1.f;
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float divx = 1.0f / dir.x;
	if (divx >= 0.0f) {
		tmin = (min.x - org.x) * divx;
		tmax = (max.x - org.x) * divx;
	}
	else {
		tmin = (max.x - org.x) * divx;
		tmax = (min.x - org.x) * divx;
	}
	if (tmax < 0.0f)
		return -1.f;

	float divy = 1.0f / dir.y;
	if (divy >= 0.0f) {
		tymin = (min.y - org.y) * divy;
		tymax = (max.y - org.y) * divy;

	}
	else {
		tymin = (max.y - org.y) * divy;
		tymax = (min.y - org.y) * divy;
	}
	if ((tymax < 0.0f) || (tmin > tymax) || (tymin > tmax))
		return -1.f;

	if (tymin > tmin)tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	float divz = 1.0f / dir.z;
	if (divz >= 0.0f) {
		tzmin = (min.z - org.z) * divz;
		tzmax = (max.z - org.z) * divz;
	}
	else {
		tzmin = (max.z - org.z) * divz;
		tzmax = (min.z - org.z) * divz;
	}
	if ((tzmax < 0.0f) || (tmin > tzmax) || (tzmin > tmax))
		return -1.f;
	glm::vec3 vec = (((min + max) / 2.0f) - org);
	dist = glm::length(vec);
	return dist;
}

float RAY::Intersect(BSPHERE bSphere)
{
	vec3 center = bSphere.center;
	vec3 oc = org - center;
	auto a = glm::dot(dir, dir);
	auto b = 2.f * glm::dot(oc, dir);
	auto c = glm::dot(oc, oc) - bSphere.radius * bSphere.radius;
	auto discriminant = b * b - 4 * a * c;
	return discriminant;
}
