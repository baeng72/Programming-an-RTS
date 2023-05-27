//////////////////////////////////////////////////////////////
// Example 4.12: Calculating Tile Cost						//
// Written by: C. Granberg, 2005							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "heightMap.h"
#include "terrain.h"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

struct PARTICLE {
	D3DXVECTOR3 position;
	D3DCOLOR color;
	static const DWORD FVF;
};

const DWORD PARTICLE::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

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
	TERRAIN m_terrain;

	float m_angle, m_radius;
	bool m_wireframe;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;

	//Vertex buffer for points
	IDirect3DVertexBuffer9* m_pVb;
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
	m_angle = m_radius = 0.0f;
	m_wireframe = false;
	m_pVb = NULL;

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
	m_mainWindow = CreateWindow("D3DWND", "Example 4.12: Calculating Tile Cost", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	LoadObjectResources(m_pDevice);

	m_terrain.Init(m_pDevice, INTPOINT(100, 100));
	m_radius = 100.0f;

	// Initialize particles
	if (m_pVb != NULL)
	{
		m_pVb->Release();
		m_pVb = NULL;
	}

	if (FAILED(m_pDevice->CreateVertexBuffer(m_terrain.m_size.x * m_terrain.m_size.y * sizeof(PARTICLE),
		D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY,
		PARTICLE::FVF, D3DPOOL_DEFAULT, &m_pVb, 0)))
		debug.Print("Failed to create particle vertex buffer");

	PARTICLE* v = NULL;
	m_pVb->Lock(0, 0, (void**)&v, D3DLOCK_DISCARD);

	for (int y = 0; y < m_terrain.m_size.y; y++)
		for (int x = 0; x < m_terrain.m_size.x; x++)
		{
			float prc = m_terrain.GetTile(x, y)->m_cost;
			int red = 255 * prc;
			int green = 255 * (1.0f - prc);

			v->color = D3DCOLOR_ARGB(255, red, green, 0);
			v->position = D3DXVECTOR3(x, m_terrain.GetTile(x, y)->m_height + 0.1f, -y);
			v++;
		}

	m_pVb->Unlock();

	//Set sampler state
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Control camera
	m_angle += deltaTime * 0.5f;
	D3DXMATRIX  matWorld, matView, matProj;
	D3DXVECTOR2 centre = D3DXVECTOR2(50.0f, 50.0f);
	D3DXVECTOR3 Eye = D3DXVECTOR3(centre.x + cos(m_angle) * m_radius, m_radius, -centre.y + sin(m_angle) * m_radius);
	D3DXVECTOR3 Lookat = D3DXVECTOR3(centre.x, 0.0f, -centre.y);

	D3DXMatrixIdentity(&matWorld);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &Eye, &Lookat, &up);
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.3333f, 1.0f, 1000.0f);

	m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	if (KEYDOWN('W'))
	{
		m_wireframe = !m_wireframe;
		Sleep(300);
	}
	else if (KEYDOWN(VK_SPACE))
	{
		m_terrain.GenerateRandomTerrain(3);

		if (FAILED(m_pDevice->CreateVertexBuffer(m_terrain.m_size.x * m_terrain.m_size.y * sizeof(PARTICLE), D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY, PARTICLE::FVF, D3DPOOL_DEFAULT, &m_pVb, 0)))
			debug.Print("Failed to create particle vertex buffer");

		PARTICLE* v = NULL;
		m_pVb->Lock(0, 0, (void**)&v, D3DLOCK_DISCARD);

		for (int y = 0; y < m_terrain.m_size.y; y++)
			for (int x = 0; x < m_terrain.m_size.x; x++)
			{
				float prc = m_terrain.GetTile(x, y)->m_cost;
				int red = 255 * prc;
				int green = 255 * (1.0f - prc);

				v->color = D3DCOLOR_ARGB(255, red, green, 0);

				v->position = D3DXVECTOR3(x, m_terrain.GetTile(x, y)->m_height + 0.1f, -y);
				v++;
			}

		m_pVb->Unlock();
	}
	else if (KEYDOWN(VK_ADD) && m_radius < 200.0f)
	{
		m_radius += deltaTime * 30.0f;
	}
	else if (KEYDOWN(VK_SUBTRACT) && m_radius > 5.0f)
	{
		m_radius -= deltaTime * 30.0f;
	}
	else if (KEYDOWN(VK_ESCAPE))
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
		if (m_wireframe)m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		m_terrain.Render();

		if (m_pVb != NULL)
		{
			D3DXMATRIX world;
			D3DXMatrixIdentity(&world);
			m_pDevice->SetTransform(D3DTS_WORLD, &world);

			m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE, true);

			m_pDevice->SetRenderState(D3DRS_POINTSIZE, FtoDword(0.5f));
			m_pDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDword(0.0f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_A, 0);
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_B, FtoDword(0.0f));
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_C, FtoDword(1.0f));
			m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

			m_pDevice->SetTexture(0, NULL);
			m_pDevice->SetFVF(PARTICLE::FVF);
			m_pDevice->SetStreamSource(0, m_pVb, 0, sizeof(PARTICLE));
			m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_terrain.m_size.x * m_terrain.m_size.y);
		}

		RECT r[] = { {10, 10, 0, 0}, {10, 30, 0, 0}, {10, 50, 0, 0} };
		m_pFont->DrawText(NULL, "W: Toggle Wireframe", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "+/-: Zoom In/Out", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "SPACE: Randomize Terrain", -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

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
		m_terrain.Release();
		UnloadObjectResources();

		if (m_pVb != NULL)
			m_pVb->Release();

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