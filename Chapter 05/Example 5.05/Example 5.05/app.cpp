//////////////////////////////////////////////////////////////
// Example 5.5: Viewport Rendering							//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "mesh.h"
#include "object.h"
#include "camera.h"

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
	MESH* m_pTrack;
	std::vector<OBJECT> m_cars;

	D3DLIGHT9 m_light;
	D3DRECT m_destRects[2];
	D3DRECT m_srcRects[2];
	D3DRECT m_currentRects[2];

	int m_viewConfig;
	float m_viewPortPrc;
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
	m_pTrack = NULL;
	m_viewConfig = 0;
	m_viewPortPrc = 0.0f;

	//Setup viewports
	m_srcRects[0].x1 = 5;
	m_srcRects[0].x2 = 395;
	m_srcRects[0].y1 = 35;
	m_srcRects[0].y2 = 595;

	m_srcRects[1].x1 = 405;
	m_srcRects[1].x2 = 795;
	m_srcRects[1].y1 = 35;
	m_srcRects[1].y2 = 595;

	m_currentRects[0] = m_destRects[0] = m_srcRects[0];
	m_currentRects[0] = m_destRects[1] = m_srcRects[1];

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
	m_mainWindow = CreateWindow("D3DWND", "Example 5.5: Viewport Rendering", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	//Load objects
	LoadObjectResources(m_pDevice);

	m_pTrack = new MESH("objects/track.x", m_pDevice);
	m_cars.push_back(OBJECT(0, D3DXVECTOR3(0.0f, 0.0f, 0.5f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1.6f));
	m_cars.push_back(OBJECT(1, D3DXVECTOR3(0.0f, 0.0f, -1.5f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), -1.6f));
	m_cars[1].m_activeCam = 1;

	//Create m_light
	::ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = D3DLIGHT_DIRECTIONAL;
	m_light.Ambient = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Diffuse = D3DXCOLOR(0.9, 0.9, 0.9, 1.0f);
	m_light.Specular = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Direction = D3DXVECTOR3(0.5f, -0.6f, 0.0f);
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

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
	//Control camera
	D3DXMATRIX  matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	for (int i = 0; i < m_cars.size(); i++)
		m_cars[i].Update(deltaTime);

	if (KEYDOWN(VK_F1))		//Change Camera 1
	{
		m_cars[0].m_activeCam++;
		if (m_cars[0].m_activeCam >= m_cars[0].m_cameras.size())
			m_cars[0].m_activeCam = 0;
		Sleep(300);
	}
	else if (KEYDOWN(VK_F2))	//Change Camera 2
	{
		m_cars[1].m_activeCam++;
		if (m_cars[1].m_activeCam >= m_cars[0].m_cameras.size())
			m_cars[1].m_activeCam = 0;
		Sleep(300);
	}
	else if (KEYDOWN(VK_SPACE))		//Change Viewport settings
	{
		m_viewConfig++;
		if (m_viewConfig >= 3)
			m_viewConfig = 0;

		if (m_viewConfig == 0)		//Split X
		{
			m_destRects[0].x1 = 5;
			m_destRects[0].x2 = 395;
			m_destRects[0].y1 = 35;
			m_destRects[0].y2 = 595;
			m_destRects[1].x1 = 405;
			m_destRects[1].x2 = 795;
			m_destRects[1].y1 = 35;
			m_destRects[1].y2 = 595;
		}
		else if (m_viewConfig == 1)		//Split Y
		{
			m_destRects[0].x1 = 5;
			m_destRects[0].x2 = 795;
			m_destRects[0].y1 = 35;
			m_destRects[0].y2 = 305;
			m_destRects[1].x1 = 5;
			m_destRects[1].x2 = 795;
			m_destRects[1].y1 = 325;
			m_destRects[1].y2 = 595;
		}
		else if (m_viewConfig == 2)		//Big 1, Small 2
		{
			m_destRects[0].x1 = 30;
			m_destRects[0].x2 = 770;
			m_destRects[0].y1 = 60;
			m_destRects[0].y2 = 570;
			m_destRects[1].x1 = 650;
			m_destRects[1].x2 = 790;
			m_destRects[1].y1 = 35;
			m_destRects[1].y2 = 175;
		}

		m_viewPortPrc = 0.0f;
		m_srcRects[0] = m_currentRects[0];
		m_srcRects[1] = m_currentRects[1];
		Sleep(300);
	}
	else if (KEYDOWN(VK_ESCAPE))
		Quit();

	//Change viewport rectangles
	m_viewPortPrc += deltaTime;
	if (m_viewPortPrc > 1.0f)m_viewPortPrc = 1.0f;

	//Linear interpolation between the two rectangles
	for (int i = 0; i < 2; i++)
	{
		m_currentRects[i].x1 = m_srcRects[i].x1 - (m_srcRects[i].x1 * m_viewPortPrc) + (m_destRects[i].x1 * m_viewPortPrc);
		m_currentRects[i].x2 = m_srcRects[i].x2 - (m_srcRects[i].x2 * m_viewPortPrc) + (m_destRects[i].x2 * m_viewPortPrc);
		m_currentRects[i].y1 = m_srcRects[i].y1 - (m_srcRects[i].y1 * m_viewPortPrc) + (m_destRects[i].y1 * m_viewPortPrc);
		m_currentRects[i].y2 = m_srcRects[i].y2 - (m_srcRects[i].y2 * m_viewPortPrc) + (m_destRects[i].y2 * m_viewPortPrc);
	}

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	// Clear the viewport
	D3DVIEWPORT9 v;
	v.X = v.Y = 0;
	v.Width = 800;
	v.Height = 600;
	v.MaxZ = 1.0f;
	v.MinZ = 0.0f;

	m_pDevice->SetViewport(&v);

	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

	RECT r[] = { {340, 10, 0, 0}, {40, 10, 0, 0}, {640, 10, 0, 0} };
	m_pFont->DrawText(NULL, "Space: Viewport", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
	m_pFont->DrawText(NULL, "F1: Camera 1", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
	m_pFont->DrawText(NULL, "F2: Camera 2", -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		//For each car
		for (int c = 0; c < m_cars.size(); c++)
		{
			//Set and clear viewport
			v.X = m_currentRects[c].x1;
			v.Y = m_currentRects[c].y1;
			v.Width = m_currentRects[c].x2 - m_currentRects[c].x1;
			v.Height = m_currentRects[c].y2 - m_currentRects[c].y1;
			m_pDevice->SetViewport(&v);
			m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff4444ff, 1.0f, 0L);

			m_cars[c].UpdateCameras();

			D3DXMATRIX world;
			D3DXMatrixIdentity(&world);
			m_pDevice->SetTransform(D3DTS_WORLD, &world);

			//Render track
			m_pTrack->Render();

			//Render both cars
			for (int i = 0; i < m_cars.size(); i++)
				m_cars[i].Render();
		}

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
		UnloadObjectResources();

		if (m_pTrack != NULL)m_pTrack->Release();

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