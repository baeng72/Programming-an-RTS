#pragma once
#include <common.h>
#include "intpoint.h"

class MOUSE : public INTPOINT {
	Renderer::RenderDevice* _pdevice;
	Window* _pwindow;
	Rect _viewport;
	std::vector<std::unique_ptr<Renderer::Texture>> _textures;
	std::unique_ptr<Renderer::Sprite> _sprite;
	float yscroll;
public:
	float _speed;
	int		_type;
public:
	MOUSE();
	~MOUSE();
	void Init(Renderer::RenderDevice*pdevice,Window* pwindow);
	bool ClickLeft();
	bool ClickRight();
	bool WheelUp();
	bool WheelDown();
	bool Over(Rect& dst);
	bool PressInRect(Rect& dst);
	void Update();
	void Paint();
};