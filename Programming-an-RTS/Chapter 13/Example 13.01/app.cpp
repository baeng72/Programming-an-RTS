#include <common.h>
#include "intpoint.h"
#include "mouse.h"

class APPLICATION : public Core::Application {
	std::unique_ptr<Renderer::RenderDevice> _device;
	std::unique_ptr<Core::ThreadPool> _threads;
	std::shared_ptr<Renderer::ShaderManager> _shadermanager;
	std::unique_ptr<Renderer::Font> _font;
	std::unique_ptr<Renderer::Sprite> _sprite;
	std::unique_ptr<Renderer::Texture> _spriteTexture;
	std::unique_ptr<Sound::SoundPlayer> _snd;
	std::vector<size_t> _sndids;
	MOUSE _mouse;
public:
	APPLICATION();
	bool Init(int width, int height, const char* title);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

};

APPLICATION::APPLICATION() {
	
	srand(693278906);
	
	Core::ResourcePath::SetProjectPath("Chapter 13/Example 13.01");
	

}

bool APPLICATION::Init(int width, int height, const char* title) {
	LOG_INFO("Application::Init() thread id: {0}", std::this_thread::get_id());
	if (!Application::Init(width, height, title))
		return false;

	_threads = std::make_unique<Core::ThreadPool>(8);
	_device.reset(Renderer::RenderDevice::Create(GetWindow().GetNativeHandle()));
	_device->EnableDepthBuffer(true);
	_device->EnableLines(true);
	_device->EnableGeometry(true);
	_device->SetVSync(false);
	_device->Init();
	_device->SetClearColor(0.f, 0.f, 0.f, 1.f);
	_font.reset(Renderer::Font::Create());
	_font->Init(_device.get(), Core::ResourcePath::GetFontPath("arialn.ttf"), 18);
	_shadermanager.reset(Renderer::ShaderManager::Create(_device.get()));
	_snd.reset(Sound::SoundPlayer::Create());
	
	_sprite.reset(Renderer::Sprite::Create(_device.get()));
	
	
	_spriteTexture.reset(Renderer::Texture::Create(_device.get(), Core::ResourcePath::GetTexturePath("sounds.jpg"), vec2(-1)));
	
	_mouse.Init(_device.get(), _shadermanager, GetWindowPtr());

	_sndids.resize(3);
	std::vector<const char*> sndnames = { "sound1.wav","sound2.wav","sound3.wav" };
	for (int i = 0; i < 3; i++) {
		_sndids[i] = _snd->loadSong(Core::ResourcePath::GetSoundPath(sndnames[i]));
	}

	return true;
}

void APPLICATION::Update(float deltaTime) {
	_mouse.Update();
	
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();
	if (_mouse.WheelDown()) {
		float mv = std::min(1.f,std::max(_snd->getMasterVolume() + 0.005f,0.f));

		_snd->setMasterVolume(mv);
	}
	if (_mouse.WheelUp()) {
		float mv = std::min(1.f, std::max(_snd->getMasterVolume() - 0.005f, 0.f));
		_snd->setMasterVolume(mv);
	}
}

void APPLICATION::Render() {
	_device->StartRender();

	char buffer[64];

	sprintf(buffer, "Sound Volumn: %f (Mouse Wheel)", _snd->getMasterVolume() * 10.f);

	
	_font->Draw(buffer, 10, 10);	

	Rect dst[] = { {136,200,256,328},{336,200,464,328},{536,200,664,328} };
	for (int i = 0; i < 3; i++) {
		Rect src = { i * 128,0,i * 128+128,128 };
		if (_mouse.Over(dst[i])) {
			src.top = 128; src.bottom = 256;
		}
		vec3 pos = vec3(dst[i].left, dst[i].top, 0.f);
		_sprite->Draw(_spriteTexture.get(), src, pos, Color(1.f, 1.f, 1.f, 1.f));
		if (_mouse.PressInRect(dst[i])) {
			_snd->play(_sndids[i]);
			_mouse.DisableInput(100);
		}
		
	}
	_font->Render();
	_mouse.Paint();


	_device->EndRender();
}

void APPLICATION::Quit() {
	SetRunning(false);
}

void APPLICATION::Cleanup() {
	_threads->Stop();
	
	_device->Wait();
}

void AppMain() {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(109020);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 13.1: Playing Sounds")) {
		app.Run();
	}

}