//////////////////////////////////////////////////////////////
// Example 13.1: Playing Sounds								//
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
	MOUSE m_mouse;
	SOUND m_sound;
	LPD3DXSPRITE m_pSprite;
	IDirect3DTexture9* m_pSoundTexture;

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
	m_pSoundTexture = NULL;
	m_pSprite = NULL;

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
	m_mainWindow = CreateWindow("D3DWND", "Example 13.1: Playing Sounds", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	//Load texture
	D3DXCreateSprite(m_pDevice, &m_pSprite);
	D3DXCreateTextureFromFile(m_pDevice, "textures/sounds.jpg", &m_pSoundTexture);

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Update mouse
	m_mouse.Update();

	//Change mouse
	if (m_mouse.WheelUp())m_sound.SetMasterVolume(m_sound.GetMasterVolume() + 0.05f);
	if (m_mouse.WheelDown())m_sound.SetMasterVolume(m_sound.GetMasterVolume() - 0.05f);


	if (KEYDOWN(VK_ESCAPE))
		Quit();

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	// Clear the viewport
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);


	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		char number[50];
		std::string sizeText = "Sound Volume: ";
		//sizeText += _itoa(m_sound.GetMasterVolume() * 10.0f, number, 10);
		_itoa_s(m_sound.GetMasterVolume() * 10.0f, number, sizeof(number), 10);
		sizeText += number;
		sizeText += "     (Mouse Wheel)";
		RECT rc = { 10, 10, 0, 0 };
		m_pFont->DrawText(NULL, sizeText.c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		m_pSprite->Begin(0);

		RECT dest[] = { {136, 200, 264, 328}, {336, 200, 464, 328}, {536, 200, 664, 328} };

		for (int i = 0; i < 3; i++)
		{
			RECT src = { i * 128, 0, i * 128 + 128, 128 };
			if (m_mouse.Over(dest[i])) { src.top = 128; src.bottom = 256; }
			D3DXVECTOR3 pos = D3DXVECTOR3(dest[i].left, dest[i].top, 0.0f);
			m_pSprite->Draw(m_pSoundTexture, &src, NULL, &pos, 0xffffffff);

			if (m_mouse.PressInRect(dest[i]))
			{
				m_sound.PlaySound(i, false);
				m_mouse.DisableInput(300);
			}
		}

		m_pSprite->End();

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
		if (m_pSprite)m_pSprite->Release();
		if (m_pSoundTexture)m_pSoundTexture->Release();

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