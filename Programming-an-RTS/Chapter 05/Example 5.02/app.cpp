#pragma once
#include <common.h>
#include "mouse.h"
class APPLICATION : public Core::Application {
	
	std::unique_ptr<Renderer::RenderDevice> _device;		
	std::unique_ptr<Renderer::Font> _font;
	MOUSE _mouse;
	Rect _rects[4];
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	
}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init()");
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->Init();
	_device->SetClearColor(1.f, 1.f, 1.f, 1.f);

	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf", 18);

	_mouse.Init(_device.get(),GetWindowPtr());
	//setup rectangles
	
	_rects[0] = { 200,100,350,250 };//Red
	_rects[1] = { 450,100,600,250 };//Green
	_rects[2] = { 200,350,350,500 };//Blue
	_rects[3] = { 450,350,600,500 };//Yellow
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	_mouse.Update();
	
	
	//change mouse
	if (_mouse.WheelUp() && _mouse._speed < 3.f)
		_mouse._speed += 0.1f;
	if (_mouse.WheelDown() && _mouse._speed > 0.3f)
		_mouse._speed -= 0.1f;

	if (_mouse.PressInRect(_rects[0]))
		_mouse._type = 1;		//red square
	if (_mouse.PressInRect(_rects[1]))
		_mouse._type = 2;		//green square
	if (_mouse.PressInRect(_rects[2]))
		_mouse._type = 3;		//blue square
	if (_mouse.PressInRect(_rects[3]))
		_mouse._type = 4;		//yellow square

	char buffer[64];
	sprintf_s(buffer, "%d Mouse Wheel", (int)(_mouse._speed * 10.f));

	_font->Draw(buffer, 10, 10, vec4(0.f, 0.f, 0.f, 1.f));
	wsprintf(buffer, "Mouse Pos: (%d,%d)", _mouse.x, _mouse.y);
	_font->Draw(buffer, 10, 30, Color(0.f, 0.f, 0.f, 1.f));

}

void APPLICATION::Render() {	
	//using DirectX LHS coordinate system.
	

	_device->StartRender();		
	_font->Render();
	
	
	Color rectCols[] = { Color(0.5f,0.f,0.f,1.f),Color(0.f,0.5f,0.f,1.f),Color(0.f,0.f,0.5f,1.f),Color(0.5f,0.5f,0.f,1.f) };
	Color rectColsOver[] = { Color(1.f,0.f,0.f,1.f),Color(0.f,1.f,0.f,1.f),Color(0.f,0.f,1.f,1.f),Color(1.f,1.f,0.f,1.f) };

	//Draw color rectangles
	for (int i = 0; i < 4; i++) {
		Rect r = _rects[i];
		if (_mouse.Over(r))
			_device->Clear(r, rectColsOver[i]);
		else
			_device->Clear(r, rectCols[i]);
	}
	
	Rect r{ 0,0,10,10 };
	_device->Clear(r, Color(.5f, .5f, .5f, 1.f));
	Rect r2{ r.left,_height - r.Height(),10,_height-r.top };
	_device->Clear(r2, Color(.5f, .5f, .5f, 1.f));
	_mouse.Paint();
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
	
}

void APPLICATION::Cleanup() {
	_device->Wait();
	
}

void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 5.2: Mouse Example")) {
		app.Run();
	}
	
}
