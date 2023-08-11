#pragma once
#include <common.h>

class APPLICATION : public Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Renderer::Font> _font;
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
	_device->Init();
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf",40);
	float fw, fh;
	_font->GetTextSize("Hello World!", fw, fh);
	_font->Draw("Hello World!", width/2-(int)fw/2, height/2-(int)fh/2);
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
}

void APPLICATION::Render() {
	_device->StartRender();
	
	_font->Render();
	
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

int main() {
	APPLICATION app;
	if (app.Init(640, 480, "Example 3.01: Application Framework")) {
		app.Run();
	}
	return 0;
}
