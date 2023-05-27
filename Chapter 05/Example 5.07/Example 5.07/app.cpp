//////////////////////////////////////////////////////////////
// Example 5.7: Level-of-Detail Example						//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mouse.h"
#include "object.h"
#include "camera.h"

struct VERTEX
{
	VERTEX(D3DXVECTOR3 p, D3DCOLOR c) { pos = p; color = c; }

	D3DXVECTOR3 pos;
	D3DCOLOR color;
	static const DWORD FVF;
};

const DWORD VERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;


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
	std::vector<OBJECT> m_mechs;
	std::vector<VERTEX> m_lines;
	MOUSE m_mouse;
	CAMERA m_camera;

	ID3DXLine* m_pLine;
	D3DLIGHT9 m_light;
	DWORD m_time;
	int m_fps, m_lastFps;
	bool m_lodOn;
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
	m_pFont = NULL;
	m_pLine = NULL;
	m_fps = m_lastFps = 0;
	m_time = GetTickCount();
	m_lodOn = true;

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
	m_mainWindow = CreateWindow("D3DWND", "Example 5.7: Level-of-Detail Example", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	//Create m_light
	::ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = D3DLIGHT_DIRECTIONAL;
	m_light.Ambient = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Diffuse = D3DXCOLOR(0.9, 0.9, 0.9, 1.0f);
	m_light.Specular = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

	//Set sampler state
	for (int i = 0; i < 4; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	//Init camera
	m_camera.Init(m_pDevice);
	m_camera.m_radius = 100.0f;

	//Load objects
	LoadObjectResources(m_pDevice);

	//Create 2D Line
	D3DXCreateLine(m_pDevice, &m_pLine);


	//Add m_mechs
	for (int y = -12; y < 12; y++)
		for (int x = -12; x < 12; x++)
		{
			if (rand() % 3 == 0)
			{
				float a = (rand() % (int)((D3DX_PI * 2.0f) * 1000)) / 1000.0f;
				m_mechs.push_back(OBJECT(MECH, D3DXVECTOR3(x * 15.0f, 0.0f, -y * 15.0f), D3DXVECTOR3(0.0f, a, 0.0f)));
			}
		}

	//Create m_lines
	for (int i = -12; i <= 12; i++)
	{
		m_lines.push_back(VERTEX(D3DXVECTOR3(-180.0f, 0.0f, -i * 15.0f), 0xff000000));
		m_lines.push_back(VERTEX(D3DXVECTOR3(180.0f, 0.0f, -i * 15.0f), 0xff000000));
		m_lines.push_back(VERTEX(D3DXVECTOR3(i * 15.0f, 0.0f, -180.0f), 0xff000000));
		m_lines.push_back(VERTEX(D3DXVECTOR3(i * 15.0f, 0.0f, 180.0f), 0xff000000));
	}


	//Init mouse
	m_mouse.InitMouse(m_pDevice, m_mainWindow);

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

	//Update camera
	m_camera.Update(m_mouse, deltaTime);

	if (KEYDOWN(VK_SPACE))
	{
		Sleep(200);
		m_lodOn = !m_lodOn;
	}
	else if (KEYDOWN(VK_ESCAPE))
		Quit();

	return S_OK;
}


HRESULT APPLICATION::Render()
{
	// Clear the viewport
	D3DVIEWPORT9 v, v2;
	v.X = 0;
	v.Y = 0;
	v.Width = 800;
	v.Height = 600;
	v.MaxZ = 1.0f;
	v.MinZ = 0.0f;
	v2 = v;
	m_pDevice->SetViewport(&v);
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);

	//FPS Calculation
	m_fps++;
	if (GetTickCount() - m_time > 1000)
	{
		m_lastFps = m_fps;
		m_fps = 0;
		m_time = GetTickCount();
	}

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		//Draw m_lines
		m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
		m_pDevice->SetFVF(VERTEX::FVF);

		for (int i = 0; i < m_lines.size(); i += 2)
			m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &m_lines[i], sizeof(VERTEX));

		m_pDevice->SetRenderState(D3DRS_LIGHTING, true);

		//Draw Mechs
		long noFaces = 0;
		int noObjects = 0;

		for (int i = 0; i < m_mechs.size(); i++)
		{
			if (m_lodOn)
				m_mechs[i].Render(&m_camera, noFaces, noObjects);
			else m_mechs[i].Render(NULL, noFaces, noObjects);
		}

		//Top menu
		v.X = 0;
		v.Y = 0;
		v.Width = 800;
		v.Height = 70;
		m_pDevice->SetViewport(&v);
		m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);

		//Number of Mechs
		RECT r[] = { {500, 10, 0, 0}, {500, 30, 0, 0}, {500, 50, 0, 0},
					{10, 10, 0, 0}, {10, 30, 0, 0}, {10, 50, 0, 0} };

		char number[50];
		std::string text = "Num Mechs in screen: ";
		//text += _itoa(noObjects, number, 10);
		_itoa_s(noObjects, number, sizeof(number), 10);
		text += number;
		text += " / ";
		//text += _itoa(m_mechs.size(), number, 10);
		_itoa_s(m_mechs.size(), number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		//Number of Polygons
		text = "Num Total Polygons: ";
		//text += _itoa(noFaces, number, 10);
		_itoa_s(noFaces, number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		//FPS
		text = "FPS: ";
		//text += _itoa(m_lastFps, number, 10);
		_itoa_s(m_lastFps, number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		//Controls
		m_pFont->DrawText(NULL, "Mouse Wheel: Change Camera Radius", -1, &r[3], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "Arrows: Change Camera Angle", -1, &r[4], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		if (m_lodOn)
			m_pFont->DrawText(NULL, "Space: LOD & Culling = On", -1, &r[5], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		else m_pFont->DrawText(NULL, "Space: LOD & Culling = Off", -1, &r[5], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		//Restore viewport
		m_pDevice->SetViewport(&v2);

		D3DXVECTOR2 l[] = { D3DXVECTOR2(0.0f, 70.0f), D3DXVECTOR2(800.0f, 70.0f) };
		m_pLine->Begin();
		m_pLine->SetWidth(4.0f);
		m_pLine->Draw(l, 2, 0xff000000);
		m_pLine->End();

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
		m_pFont->Release();
		m_pLine->Release();
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