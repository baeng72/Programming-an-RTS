#include "mouse.h"

MOUSE::MOUSE() {
	_type = 0;
	_speed = 1.5f;
	

}

MOUSE::~MOUSE() {
	_pwindow->ShowCursor(true);
	_sprite.reset();
	_textures.clear();
}

void MOUSE::Init(Renderer::RenderDevice* pdevice,Core::Window* pwindow) {
	_pdevice = pdevice;
	_pwindow = pwindow;
	pwindow->ShowCursor(false);
	std::unique_ptr<Renderer::Image> image = std::unique_ptr<Renderer::Image>(Renderer::Image::Create(Core::ResourcePath::GetCursorPath("cursor.png")));
	uint32_t* pixels = (uint32_t*)image->GetPixels();
	int width, height, channels;
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
		_textures[i] = std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, 20, 20, Renderer::TextureFormat::R8G8B8A8,(uint8_t*) newpixels));
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