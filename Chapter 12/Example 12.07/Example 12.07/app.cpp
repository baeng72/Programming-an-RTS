//////////////////////////////////////////////////////////////
// Example 12.7: Constructing Buildings						//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"

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
	MESH m_build1, m_build2;
	float m_buildPrc;

	D3DLIGHT9 m_light;
	HWND m_mainWindow;
};

D3DRECT SetRect(long x1, long y1, long x2, long y2)
{
	D3DRECT r;
	r.x1 = x1;
	r.y1 = y1;
	r.x2 = x2;
	r.y2 = y2;
	return r;
}

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
	m_mainWindow = CreateWindow("D3DWND", "Example 12.7: Constructing Buildings", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	//Set sampler state
	for (int i = 0; i < 4; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	m_build1.Load("meshes/townhall_b.x", m_pDevice);
	m_build2.Load("meshes/townhall.x", m_pDevice);
	m_buildPrc = 0.0f;

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	m_buildPrc += deltaTime * 0.07f;
	if (m_buildPrc > 1.5f)m_buildPrc = 0.0f;

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
	D3DXMATRIX view, proj, world, identity;
	D3DXVECTOR3 eye = D3DXVECTOR3(15.0f, 30.0f, -40.0f);
	D3DXVECTOR3 center = D3DXVECTOR3(0.0f, 11.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&view, &eye, &center, &up);
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.3f, 1.33333f, 0.01f, 1000.0f);
	D3DXMatrixIdentity(&identity);

	m_pDevice->SetTransform(D3DTS_WORLD, &identity);
	m_pDevice->SetTransform(D3DTS_VIEW, &view);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		if (m_buildPrc < 1.0f)
		{
			m_build1.Render(m_buildPrc);

			//Progressbar
			D3DRECT r = SetRect(10, 560, 790, 590);
			m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);
			r = SetRect(12, 562, 788, 588);
			m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
			r = SetRect(12, 562, 12 + 774 * m_buildPrc, 588);
			m_pDevice->Clear(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff00ff00, 1.0f, 0);
		}
		else m_build2.Render();

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
		m_build1.Release();
		m_build2.Release();

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