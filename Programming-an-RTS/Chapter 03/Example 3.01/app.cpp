#pragma once
#include <common.h>

class APPLICATION : public Core::Application {
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

bool APPLICATION::Init(int width, int height, const char* title){
	LOG_INFO("Application::Init()");
	
	if (!Application::Init(width, height, title))
		return false;

	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->SetVSync(false);
	_device->Init();
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), "../../../../Resources/Fonts/arialn.ttf",40);
	_time = _device->GetCurrentTicks();
	return true;
}

void APPLICATION::Update(float deltaTime) {
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
}

void APPLICATION::Render() {
	
	_device->StartRender();
	float fw, fh;
	_font->GetTextSize("Hello World!", fw, fh);
	_font->Draw("Hello World!", _width / 2 - (int)fw / 2, _height / 2 - (int)fh / 2);
	_font->Render();
	
	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {

}

void AppMain(){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//EASY_PROFILER_ENABLE;
	
	APPLICATION app;
	if (app.Init(800, 600, "Example 3.01: Application Framework")) {
		app.Run();
	}
	/*if(Core::GetAPI()==Core::API::GL)
		profiler::dumpBlocksToFile("Example3.01-GL.prof");
	else
		profiler::dumpBlocksToFile("Example3.01-Vulkan.prof");*/
	
}
