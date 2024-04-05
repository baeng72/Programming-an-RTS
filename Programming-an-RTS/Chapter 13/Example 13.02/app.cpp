#include <common.h>
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
	float _volumes[2];
	int _playing;

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
	
	Core::ResourcePath::SetProjectPath("Chapter 13/Example 13.02");

	_volumes[0] = 0.f;
	_volumes[1] = 0.f;
	_playing = -1;
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

	_sprite.reset(Renderer::Sprite::Create(_device.get()));


	_spriteTexture.reset(Renderer::Texture::Create(_device.get(), Core::ResourcePath::GetTexturePath("mp3Player.bmp"), vec2(-1)));

	_snd.reset(Sound::SoundPlayer::Create());
	_sndids.resize(2);
	std::vector<const char*> sndnames = { "song1.mp3","song2.mp3"};
	for (int i = 0; i < 2; i++) {
		
		_sndids[i] = _snd->loadSong(Core::ResourcePath::GetMusicPath(sndnames[i]),true);
		_snd->play(_sndids[i]);
		_snd->setVolume(_sndids[i], _volumes[i]);
	}

	_mouse.Init(_device.get(), _shadermanager, GetWindowPtr());
	
	return true;
}

void APPLICATION::Update(float deltaTime) {
	_mouse.Update();
	if (IsKeyPressed(KEY_ESCAPE))
		Quit();


	for (int i = 0; i < 2; i++) {
		if (_playing == i)
			_volumes[i] += deltaTime * 0.1f;
		else {
			if (_volumes[i] > 0.5f)
				_volumes[i] -= deltaTime * 0.1f;
		}
		_volumes[i] = std::min(1.f, std::max(0.f, _volumes[i]));//clamp volume
		_snd->setVolume(_sndids[i],_volumes[i]);
			
		
	}
}

void APPLICATION::Render() {
	
	_device->StartRender();

	Rect playRects[] = { {100,250,200,350}, {500,250,600,350} };
	Rect playSrc[] = { {0,0,100,100}, {0,100,100,200} };
	Rect volSrc[] = { {100,0,150,140},{150,0,200,140} };

	for (int i = 0; i < 2; i++) {
		int src = 0;
		if (_mouse.Over(playRects[i]))
			src = 1;

		//play button
		vec3 vec = vec3(playRects[i].left, playRects[i].top, 0.f);
		_sprite->Draw(_spriteTexture.get(), playSrc[src], vec, Color(1.f));
		if (_mouse.PressInRect(playRects[i])) {
			_mouse.DisableInput(300);
			_playing = i;
		}

		//volume meter
		int v = 140 - 140 * _volumes[i];
		Rect a = { volSrc[0].left,volSrc[0].top+v,volSrc[0].right,volSrc[0].bottom };
		Rect b = { volSrc[1].left,volSrc[1].top,volSrc[1].right,volSrc[1].top + v };

		vec = vec3(playRects[i].left + 110, playRects[i].top - 20 + v, 0.f);
		_sprite->Draw(_spriteTexture.get(), a, vec, Color(1.f));
		vec = vec3(playRects[i].left + 110, playRects[i].top - 20, 0.f);
		_sprite->Draw(_spriteTexture.get(), b, vec, Color(1.f));
	}
	float width, height;
	_font->GetTextSize("Music composed and produced by Paul Houseman (www.paulhouseman.com)", width, height);
	_font->Draw("Music composed and produced by Paul Houseman (www.paulhouseman.com)", _width/2-width/2.f, 548);	
	
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
	//_CrtSetBreakAlloc(465433);
#endif
	APPLICATION app;
	if (app.Init(800, 600, "Example 13.2: Play that funky music...")) {
		app.Run();
	}

}