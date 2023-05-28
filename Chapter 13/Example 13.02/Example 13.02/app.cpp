//////////////////////////////////////////////////////////////
// Example 13.2: Play that funky music...					//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mouse.h"
#include "sound.h"

class APPLICATION
{
public:
	APPLICATION();
	HRESULT Init(HINSTANCE hInstance, int width, int height, bool windowed);
	HRESULT Update(float deltaTime);
	HRESULT Render();
	HRESULT Cleanup();
	HRESULT Quit();

private:
	IDirect3DDevice9* m_pDevice;
	IDirect3DTexture9* m_pPlayTexture;
	MOUSE m_mouse;
	SOUND m_sound;
	MP3 m_mp3Files[2];

	float m_volumes[2];
	int m_playing;
	LPD3DXSPRITE m_pSprite;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	APPLICATION app;

	if (FAILED(app.Init(hInstance, 800, 600, true)))
		return 0;

	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	int startTime = timeGetTime();

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			int t = timeGetTime();
			float deltaTime = (t - startTime) * 0.001f;

			app.Update(deltaTime);
			app.Render();

			startTime = t;
		}
	}

	app.Cleanup();

	return msg.wParam;
}

APPLICATION::APPLICATION()
{
	m_pDevice = NULL;
	m_mainWindow = 0;
	m_pPlayTexture = NULL;
	m_volumes[0] = 0.0f;
	m_volumes[1] = 0.0f;
	m_playing = -1;

	srand(GetTickCount());
}

HRESULT APPLICATION::Init(HINSTANCE hInstance, int width, int height, bool windowed)
{
	debug.Print("Application initiated");

	//Create Window Class
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)::DefWindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "D3DWND";

	//Register Class and Create new Window
	RegisterClass(&wc);
	m_mainWindow = CreateWindow("D3DWND", "Example 13.2: Play that funky music...", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
	SetCursor(NULL);
	ShowWindow(m_mainWindow, SW_SHOW);
	UpdateWindow(m_mainWindow);

	//Create IDirect3D9 Interface
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d9 == NULL)
	{
		debug.Print("Direct3DCreate9() - FAILED");
		return E_FAIL;
	}

	//Check that the Device supports what we need from it
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	//Hardware Vertex Processing or not?
	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//Check vertex & pixelshader versions
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		debug.Print("Warning - Your graphic card does not support vertex and pixelshaders version 2.0");
	}

	//Set D3DPRESENT_PARAMETERS
	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = m_mainWindow;
	d3dpp.Windowed = windowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//Create the IDirect3DDevice9
	if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_mainWindow,
		vp, &d3dpp, &m_pDevice)))
	{
		debug.Print("Failed to create IDirect3DDevice9");
		return E_FAIL;
	}

	//Release IDirect3D9 interface
	d3d9->Release();

	D3DXCreateFont(m_pDevice, 18, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont);

	//Set sampler state
	for (int i = 0; i < 4; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	//Init mouse
	m_mouse.InitMouse(m_pDevice, m_mainWindow);

	//Init Sound
	m_sound.Init(m_mainWindow);

	//Init MP3 files
	for (int i = 0; i < 2; i++)
	{
		m_mp3Files[i].Init();
		if (i == 0)m_mp3Files[i].LoadSong(L"music/song1.mp3");
		if (i == 1)m_mp3Files[i].LoadSong(L"music/song2.mp3");
		m_mp3Files[i].Play();
		m_mp3Files[i].SetVolume(m_volumes[i]);
	}

	D3DXCreateTextureFromFile(m_pDevice, "textures/mp3Player.bmp", &m_pPlayTexture);
	D3DXCreateSprite(m_pDevice, &m_pSprite);

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Control camera
	D3DXMATRIX  matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	//Update mouse
	m_mouse.Update();

	if (KEYDOWN(VK_ESCAPE))
		Quit();

	//Update Mp3 Players
	for (int i = 0; i < 2; i++)
	{
		if (m_playing == i)m_volumes[i] += deltaTime * 0.1f;
		else
		{
			if ((i == 0 && m_volumes[1] > 0.5f) || (i == 1 && m_volumes[0] > 0.5f))
				m_volumes[i] -= deltaTime * 0.1f;
		}

		if (m_volumes[i] > 1.0f)m_volumes[i] = 1.0f;
		if (m_volumes[i] < 0.0f)m_volumes[i] = 0.0f;

		m_mp3Files[i].SetVolume(m_volumes[i]);

		if (!m_mp3Files[i].IsPlaying())
			m_mp3Files[i].Play();
	}

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	// Clear the viewport
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_pSprite->Begin(0);

		RECT playRects[] = { {100, 250, 200, 350}, {500, 250, 600, 350} };
		RECT playSrc[] = { {0, 0, 100, 100}, {0, 100, 100, 200} };
		RECT volSrc[] = { {100, 0, 150, 140}, {150, 0, 200, 140} };

		for (int i = 0; i < 2; i++)
		{
			int src = 0;
			if (m_mouse.Over(playRects[i]))src = 1;

			//Play Button
			auto vec = D3DXVECTOR3(playRects[i].left, playRects[i].top, 0.0f);
			m_pSprite->Draw(m_pPlayTexture, &playSrc[src], NULL, &vec, 0xffffffff);
			if (m_mouse.PressInRect(playRects[i]))
			{
				m_mouse.DisableInput(300);
				m_playing = i;
			}

			//Volume meter
			int v = 140 - 140 * m_volumes[i];
			RECT a = { volSrc[0].left, volSrc[0].top + v, volSrc[0].right, volSrc[0].bottom };
			RECT b = { volSrc[1].left, volSrc[1].top, volSrc[1].right, volSrc[1].top + v };
			vec = D3DXVECTOR3(playRects[i].left + 110, playRects[i].top - 20 + v, 0.0f);
			m_pSprite->Draw(m_pPlayTexture, &a, NULL, &vec, 0xffffffff);
			vec = D3DXVECTOR3(playRects[i].left + 110, playRects[i].top - 20, 0.0f);
			m_pSprite->Draw(m_pPlayTexture, &b, NULL, &vec, 0xffffffff);
		}


		m_pSprite->End();


		RECT r[] = { {0, 568, 800, 600}, {4, 570, 800, 600} };
		m_pFont->DrawText(NULL, "Music composed and produced by Paul Houseman (www.paulhouseman.com)", -1, &r[0], DT_CENTER | DT_TOP | DT_NOCLIP, 0xffAAAAAA);
		m_pFont->DrawText(NULL, "Music composed and produced by Paul Houseman (www.paulhouseman.com)", -1, &r[1], DT_CENTER | DT_TOP | DT_NOCLIP, 0xff444444);

		//Draw mouse
		m_mouse.Paint();

		// End the scene.
		m_pDevice->EndScene();
		m_pDevice->Present(0, 0, 0, 0);
	}

	return S_OK;
}

HRESULT APPLICATION::Cleanup()
{
	try
	{
		m_mp3Files[0].Release();
		m_mp3Files[1].Release();

		if (m_pSprite)m_pSprite->Release();
		if (m_pPlayTexture)m_pPlayTexture->Release();

		m_pFont->Release();
		m_pDevice->Release();

		debug.Print("Application terminated");
	}
	catch (...) {}

	return S_OK;
}

HRESULT APPLICATION::Quit()
{
	::DestroyWindow(m_mainWindow);
	::PostQuitMessage(0);
	return S_OK;
}