//////////////////////////////////////////////////////////////
// Example 5.4: RTS Unit Selection Example					//
// Written by: C. Granberg, 2005							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mouse.h"
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

	void Select();
	int GetGnome();

private:
	IDirect3DDevice9* m_pDevice;
	std::vector<OBJECT> m_gnomes;
	MOUSE m_mouse;
	CAMERA m_camera;

	ID3DXLine* m_pLine;
	D3DLIGHT9 m_light;
	bool m_areaSelect;
	INTPOINT m_startSel;
	HWND m_mainWindow;
	ID3DXFont* m_pFont, * m_pFontMouse;
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
	m_areaSelect = false;
	m_pLine = NULL;

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
	m_mainWindow = CreateWindow("D3DWND", "Example 5.4: RTS Unit Selection Example", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	D3DXCreateFont(m_pDevice, 24, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial Black", &m_pFontMouse);

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

	//Load objects
	LoadObjectResources(m_pDevice);

	//Create 2D Line
	D3DXCreateLine(m_pDevice, &m_pLine);

	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "Sgt. Salty"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(2.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.6f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "Gnomad"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(2.0f, 0.0f, 2.0f), D3DXVECTOR3(0.0f, 2.6f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "Mr Finch"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(-3.0f, 0.0f, -4.0f), D3DXVECTOR3(0.0f, 1.6f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "Hanz"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(-1.0f, 0.0f, 3.0f), D3DXVECTOR3(0.0f, -0.6f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "von Wunderbaum"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(5.0f, 0.0f, -3.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "Iceman"));
	m_gnomes.push_back(OBJECT(GNOME, D3DXVECTOR3(-6.0f, 0.0f, 5.0f), D3DXVECTOR3(0.0f, -1.6f, 0.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f), "!!!"));

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

	if (KEYDOWN(VK_ESCAPE))
		Quit();

	return S_OK;
}

void APPLICATION::Select()
{
	try
	{
		if (m_mouse.ClickLeft())	// If the mouse button is pressed
		{
			for (int i = 0; i < m_gnomes.size(); i++)	//Deselect all Objects
				m_gnomes[i].m_selected = false;

			if (!m_areaSelect)		// If no area selection is in progress
			{
				//GetGnome() returns a Gnome using the mouse ray. If no 
				// gnome is found this function returns -1
				int gnome = GetGnome();

				if (gnome >= 0)
					m_gnomes[gnome].m_selected = true;
				else
				{
					m_areaSelect = true;		// if no gnome if found, 											
					m_startSel = m_mouse;		// start area selection
				}
			}
			else					//Area Selection in progress
			{
				// Create area rectangle
				INTPOINT p1 = m_startSel, p2 = m_mouse;
				if (p1.x > p2.x) { int temp = p2.x; p2.x = p1.x; p1.x = temp; }
				if (p1.y > p2.y) { int temp = p2.y; p2.y = p1.y; p1.y = temp; }
				RECT selRect = { p1.x, p1.y, p2.x, p2.y };

				//Draw selection rectangle
				D3DXVECTOR2 box[] = { D3DXVECTOR2(p1.x, p1.y), D3DXVECTOR2(p2.x, p1.y),
									 D3DXVECTOR2(p2.x, p2.y), D3DXVECTOR2(p1.x, p2.y),
									 D3DXVECTOR2(p1.x, p1.y) };

				m_pLine->SetWidth(1.0f);
				m_pLine->Begin();
				m_pLine->Draw(box, 5, 0xffffffff);
				m_pLine->End();

				//Select any gnomes inside our rectangle
				for (int i = 0; i < m_gnomes.size(); i++)
				{
					INTPOINT p = GetScreenPos(m_gnomes[i].GetPosition(), m_pDevice);
					if (p.inRect(selRect))m_gnomes[i].m_selected = true;
				}
			}
		}
		else if (m_areaSelect)		//Stop area selection
			m_areaSelect = false;

	}
	catch (...)
	{
		debug.Print("Error in APPLICATION::Select()");
	}
}

int APPLICATION::GetGnome()
{
	//Find best Gnome
	int gnome = -1;
	float bestDist = 100000.0f;
	for (int i = 0; i < m_gnomes.size(); i++)
	{
		D3DXMATRIX world = m_gnomes[i].m_meshInstance.GetWorldMatrix();
		m_pDevice->SetTransform(D3DTS_WORLD, &world);
		RAY ray = m_mouse.GetRay();
		float dist = ray.Intersect(m_gnomes[i].m_meshInstance);

		if (dist >= 0.0f && dist < bestDist)
		{
			gnome = i;
			bestDist = dist;
		}
	}

	return gnome;
}


HRESULT APPLICATION::Render()
{
	// Clear the viewport
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		for (int i = 0; i < m_gnomes.size(); i++)
			m_gnomes[i].Render();

		for (int i = 0; i < m_gnomes.size(); i++)
			m_gnomes[i].PaintSelected();

		int gnome = GetGnome();

		Select();

		//Write mouse text
		if (gnome != -1)
		{
			RECT mr[] = { {m_mouse.x + 2, m_mouse.y + 24, 0, 0}, {m_mouse.x, m_mouse.y + 22, 0, 0} };
			m_pFontMouse->DrawText(NULL, m_gnomes[gnome].m_name.c_str(), -1, &mr[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
			m_pFontMouse->DrawText(NULL, m_gnomes[gnome].m_name.c_str(), -1, &mr[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffff0000);
		}

		RECT r[] = { {10, 10, 0, 0}, {10, 30, 0, 0} };
		m_pFont->DrawText(NULL, "Mouse Wheel: Change Camera Radius", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_pFont->DrawText(NULL, "Arrows: Change Camera Angle", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

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
		m_pFontMouse->Release();

		m_gnomes.clear();

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