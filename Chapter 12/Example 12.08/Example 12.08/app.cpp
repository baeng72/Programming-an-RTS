//////////////////////////////////////////////////////////////
// Example 12.8: The Roof, the Roof, the Roof is on Fire...	//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "effect.h"
#include "mesh.h"
#include "camera.h"
#include "particles.h"

extern std::vector<EFFECT*> effects;

class APPLICATION
{
public:
	APPLICATION();
	HRESULT Init(HINSTANCE hInstance, int width, int height, bool windowed);
	HRESULT Update(float deltaTime);
	HRESULT Render();
	HRESULT Cleanup();
	HRESULT Quit();
	DWORD FtoDword(float f) { return *((DWORD*)&f); }

private:
	IDirect3DDevice9* m_pDevice;
	MESH m_building;
	CAMERA m_camera;

	DWORD m_time;
	int m_fps, m_lastFps;
	D3DLIGHT9 m_light;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	APPLICATION app;

	if (FAILED(app.Init(hInstance, 800, 600, true)))return 0;

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
	srand(GetTickCount());

	m_fps = m_lastFps = 0;
	m_time = GetTickCount();
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
	m_mainWindow = CreateWindow("D3DWND", "Example 12.8: The Roof, the Roof, the Roof is on Fire...", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	// Create m_light
	::ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = D3DLIGHT_DIRECTIONAL;
	m_light.Ambient = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Diffuse = D3DXCOLOR(0.9, 0.9, 0.9, 1.0f);
	m_light.Specular = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	D3DXVECTOR3 dir = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	D3DXVec3Normalize(&dir, &dir);
	m_light.Direction = dir;
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

	D3DXCreateFont(m_pDevice, 18, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont);

	LoadEffectResources(m_pDevice);
	LoadParticleResources(m_pDevice);

	//Add fires	
	effects.push_back(new EFFECT_FIRE(m_pDevice, D3DXVECTOR3(0.0f, 25.0f, -5.0f), D3DXVECTOR3(25.0f, 25.0f, 25.0f)));
	effects.push_back(new EFFECT_FIRE(m_pDevice, D3DXVECTOR3(-17.0f, 5.0f, -5.0f), D3DXVECTOR3(15.0f, 15.0f, 30.0f)));
	effects.push_back(new EFFECT_FIRE(m_pDevice, D3DXVECTOR3(15.0f, 0.0f, 7.0f), D3DXVECTOR3(20.0f, 20.0f, 25.0f)));

	m_building.Load("meshes/townhall.x", m_pDevice);

	m_camera.Init(m_pDevice);
	m_camera.m_radius = 100.0f;
	m_camera.m_fov = D3DX_PI * 0.25f;
	m_camera.m_focus = D3DXVECTOR3(0.0f, 25.0f, 0.0f);
	m_camera.m_beta = 0.5f;

	//Set sampler state
	for (int i = 0; i < 4; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	m_camera.Update(deltaTime);

	//Remove dead effects
	std::vector<EFFECT*>::iterator i;
	for (i = effects.begin(); i != effects.end();)
	{
		if ((*i)->isDead())
		{
			delete (*i);
			effects.erase(i);
		}
		else
		{
			(*i)->Update(deltaTime);
			i++;
		}
	}

	//Keayboard input
	if (KEYDOWN(VK_SPACE))
	{
	}
	if (KEYDOWN(VK_RETURN))
	{
	}
	else if (KEYDOWN(VK_ESCAPE))
	{
		Quit();
	}

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	//FPS Calculation
	m_fps++;
	if (GetTickCount() - m_time > 1000)
	{
		m_lastFps = m_fps;
		m_fps = 0;
		m_time = GetTickCount();
	}

	// Clear the viewport
	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	m_pDevice->SetTransform(D3DTS_WORLD, &identity);


	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_building.Render();

		//Render Effects
		for (int i = 0; i < effects.size(); i++)
			if (effects[i] != NULL)
				effects[i]->Render();

		RECT r[] = { {10, 10, 0, 0}, {720, 10, 0, 0} };
		m_pFont->DrawText(NULL, "Arrows: Rotate Camera", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		//FPS
		char number[50];
		std::string text = "FPS: ";
		//text += _itoa(m_lastFps, number, 10);
		_itoa_s(m_lastFps, number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

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
		UnloadEffectResources();
		UnloadParticleResources();

		m_building.Release();

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