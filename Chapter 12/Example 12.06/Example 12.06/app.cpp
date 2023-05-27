//////////////////////////////////////////////////////////////
// Example 12.6: Lens Flare Effect							//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "skybox.h"
#include "mouse.h"
#include "effect.h"

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
	MOUSE m_mouse;
	SKYBOX* m_pSkybox;

	HWND m_mainWindow;
	INTPOINT m_lastM;
	D3DXVECTOR3 m_rot;

	//Effects 
	std::vector<EFFECT*> m_effects;
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
	m_pSkybox = NULL;
	m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
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
	m_mainWindow = CreateWindow("D3DWND", "Example 12.6: Lens Flare Effect", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	m_mouse.InitMouse(m_pDevice, m_mainWindow);
	m_lastM = m_mouse;
	m_lastM.y += 100;

	LoadEffectResources(m_pDevice);

	m_pSkybox = new SKYBOX(m_pDevice, "textures/skybox", 300.0f);

	m_effects.push_back(new EFFECT_LENSFLARE(m_pDevice, 0, D3DXVECTOR3(-200.0f, 290.0f, 0.0f)));

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
	m_mouse.Update();

	//Remove dead effects
	std::vector<EFFECT*>::iterator i;
	for (i = m_effects.begin(); i != m_effects.end();)
	{
		if ((*i)->isDead())
		{
			delete (*i);
			m_effects.erase(i);
		}
		else
		{
			(*i)->Update(deltaTime);
			i++;
		}
	}

	//Update Camera
	if (m_mouse != m_lastM)
	{
		m_rot.y -= (m_lastM.x - m_mouse.x) * 0.01f;
		m_rot.z += (m_lastM.y - m_mouse.y) * 0.01f;
		m_lastM = m_mouse;
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
	// Clear the viewport
	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

	//Set camera
	D3DXMATRIX r;
	D3DXMatrixRotationYawPitchRoll(&r, m_rot.y, m_rot.x, m_rot.z);
	D3DXVECTOR3 focus = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	D3DXVec3TransformNormal(&focus, &focus, &r);
	D3DXVec3Normalize(&focus, &focus);

	D3DXMATRIX view, proj, world, identity;
	D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&view, &eye, &focus, &up);
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.4f, 1.33333f, 0.01f, 1000.0f);
	D3DXMatrixIdentity(&identity);

	m_pDevice->SetTransform(D3DTS_WORLD, &identity);
	m_pDevice->SetTransform(D3DTS_VIEW, &view);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_pSkybox->Render(D3DXVECTOR3(0.0f, 0.0f, 0.0f));

		//Render Effects
		for (int i = 0; i < m_effects.size(); i++)
			if (m_effects[i] != NULL)
				m_effects[i]->Render();

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
		UnloadEffectResources();

		if (m_pSkybox)delete m_pSkybox;

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