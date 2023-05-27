//////////////////////////////////////////////////////////////
// Example 4.1: Heightmaps from Images						//
// Written by: C. Granberg, 2005							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "heightMap.h"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

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
	HEIGHTMAP* m_pHeightMap;

	int m_image;
	float m_angle;
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

	return (int)msg.wParam;
}

APPLICATION::APPLICATION()
{
	m_pDevice = NULL;
	m_pHeightMap = NULL;
	m_mainWindow = 0;
	m_angle = 0.0f;
	m_image = 0;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 4.1: Heightmaps from Images", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	if (m_pHeightMap == NULL)		//Create Heightmap
	{
		//Load heightmap from file
		m_pHeightMap = new HEIGHTMAP(m_pDevice, INTPOINT(100, 100));
		if (FAILED(m_pHeightMap->LoadFromFile("images/abe.jpg")))
		{
			debug.Print("Failed to load from file");
			Quit();
		}

		//Create particles to visualize the heightmap
		if (FAILED(m_pHeightMap->CreateParticles()))
		{
			debug.Print("Failed to create particles");
			Quit();
		}
	}
	else
	{
		//Control camera
		m_angle += deltaTime * 0.5f;
		D3DXMATRIX  matWorld, matView, matProj;
		D3DXVECTOR2 centre = m_pHeightMap->GetCentre();
		D3DXVECTOR3 Eye = D3DXVECTOR3(centre.x + cos(m_angle) * centre.x * 2.0f, m_pHeightMap->m_maxHeight * 8.0f, -centre.y + sin(m_angle) * centre.y * 2.0f);
		D3DXVECTOR3 Lookat = D3DXVECTOR3(centre.x, 0.0f, -centre.y);

		D3DXMatrixIdentity(&matWorld);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &Eye, &Lookat, &up);
		D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.3333f, 1.0f, 1000.0f);

		m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		m_pDevice->SetTransform(D3DTS_VIEW, &matView);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

		//Change heightmap image
		if (KEYDOWN(VK_SPACE))
		{
			m_image++;
			if (m_image > 2)m_image = 0;
			if (m_image == 0)m_pHeightMap->LoadFromFile("images/abe.jpg");
			if (m_image == 1)m_pHeightMap->LoadFromFile("images/smiley.bmp");
			if (m_image == 2)m_pHeightMap->LoadFromFile("images/heightmap.jpg");

			m_pHeightMap->CreateParticles();
			Sleep(300);
		}
	}

	if (KEYDOWN(VK_ESCAPE))
		Quit();

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	// Clear the viewport
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		if (m_pHeightMap != NULL)m_pHeightMap->Render();

		RECT r = { 630, 10, 800, 30 };
		m_pFont->DrawText(NULL, "Space: Change Image", -1, &r, DT_LEFT | DT_NOCLIP | DT_TOP, 0xffffffff);

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
		if (m_pHeightMap != NULL)
		{
			delete m_pHeightMap;
			m_pHeightMap = NULL;
		}

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