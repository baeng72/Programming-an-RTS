//////////////////////////////////////////////////////////////
// Example 4.2: Heightmaps from Perlin Noise				//
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

	float m_angle;
	int m_size, m_amplitude;
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
	m_pHeightMap = NULL;
	m_mainWindow = 0;
	m_angle = 0.0f;
	m_pFont = NULL;
	m_size = 10;
	m_amplitude = 5;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 4.2: Heightmaps from Perlin Noise", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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
	//Create Heightmap
	if (m_pHeightMap == NULL)
	{
		//Create random heightmap
		m_pHeightMap = new HEIGHTMAP(m_pDevice, INTPOINT(100, 100));
		if (FAILED(m_pHeightMap->CreateRandomHeightMap(rand() % 2000, m_size / 10.0f, m_amplitude / 10.0f, 9)))
		{
			debug.Print("Failed to Create Random HeightMap");
			Quit();
		}

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
		D3DXVECTOR3 Eye = D3DXVECTOR3(centre.x + cos(m_angle) * centre.x * 1.5f, m_pHeightMap->m_maxHeight * 2.0f, -centre.y + sin(m_angle) * centre.y * 1.5f);
		D3DXVECTOR3 Lookat = D3DXVECTOR3(centre.x, 0.0f, -centre.y);

		D3DXMatrixIdentity(&matWorld);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &Eye, &Lookat, &up);
		D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.3333f, 1.0f, 1000.0f);

		m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		m_pDevice->SetTransform(D3DTS_VIEW, &matView);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

		if (KEYDOWN(VK_SPACE))
		{
			m_pHeightMap->CreateRandomHeightMap(rand() % 2000, m_size / 10.0f, m_amplitude / 10.0f, 9);
			m_pHeightMap->CreateParticles();
		}
	}

	if (KEYDOWN(VK_ESCAPE))
		Quit();

	if (KEYDOWN(VK_DOWN) && m_size > 1) { m_size--; Sleep(100); }
	if (KEYDOWN(VK_UP) && m_size < 20) { m_size++; Sleep(100); }
	if (KEYDOWN(VK_LEFT) && m_amplitude > 1) { m_amplitude--; Sleep(100); }
	if (KEYDOWN(VK_RIGHT) && m_amplitude < 15) { m_amplitude++; Sleep(100); }

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

		//Write HeightMap variables
		char number[50];
		std::string sizeText = "Size: ";
		//sizeText += _itoa(m_size, number, 10);
		_itoa_s(m_size, number, sizeof(number), 10);
		sizeText += number;
		sizeText += "     (UP/DOWN Arrow)";
		RECT r1 = { 110, 10, 0, 0 };
		m_pFont->DrawText(NULL, sizeText.c_str(), -1, &r1, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		sizeText = "Persistance: ";
		//sizeText += _itoa(m_amplitude, number, 10);
		_itoa_s(m_amplitude, number, sizeof(number), 10);
		sizeText += number;
		sizeText += "     (LEFT/RIGHT Arrow)";
		RECT r2 = { 110, 30, 0, 0 };
		m_pFont->DrawText(NULL, sizeText.c_str(), -1, &r2, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		RECT r3 = { 110, 50, 0, 0 };
		m_pFont->DrawText(NULL, "Redraw: SPACE", -1, &r3, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

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