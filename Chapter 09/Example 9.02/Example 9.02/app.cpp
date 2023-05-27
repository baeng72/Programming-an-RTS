//////////////////////////////////////////////////////////////
// Example 9.2: Building Example							//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "shader.h"
#include "terrain.h"
#include "camera.h"
#include "mouse.h"
#include "building.h"

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

	void Select();
	int GetBuilding();
	void AddBuildings();

private:
	IDirect3DDevice9* m_pDevice;
	TERRAIN m_terrain;
	CAMERA m_camera;
	MOUSE m_mouse;
	std::vector<BUILDING*> m_buildings;
	BUILDING* m_pBuildToPlace;

	bool m_wireframe, m_placeBuilding;
	DWORD m_time;
	int m_fps, m_lastFps, m_placeType;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;

	//Shaders
	SHADER m_unitVS, m_unitPS;
	D3DXHANDLE worldHandle, viewProjHandle, sunHandle, teamColHandle, vertexCol;

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
	m_wireframe = m_placeBuilding = false;
	srand(GetTickCount());
	m_fps = m_lastFps = 0;
	m_time = GetTickCount();
	m_pBuildToPlace = NULL;
	m_placeType = 0;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 9.2: Building Example", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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
	LoadMapObjectResources(m_pDevice);
	LoadBuildingResources(m_pDevice);

	m_terrain.Init(m_pDevice, INTPOINT(100, 100));

	m_mouse.InitMouse(m_pDevice, m_mainWindow);

	m_camera.Init(m_pDevice);
	m_camera.m_focus = D3DXVECTOR3(50, 10, -50);
	m_camera.m_fov = 0.6f;
	m_camera.m_radius = 50.0f;

	//Set sampler state
	for (int i = 0; i < 8; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	//Setup shaders
	m_unitVS.Init(m_pDevice, "shaders/lighting.vs", VERTEX_SHADER);
	worldHandle = m_unitVS.GetConstant("matW");
	viewProjHandle = m_unitVS.GetConstant("matVP");
	sunHandle = m_unitVS.GetConstant("DirToSun");
	vertexCol = m_unitVS.GetConstant("vertexCol");

	m_unitPS.Init(m_pDevice, "shaders/teamCol.ps", PIXEL_SHADER);
	teamColHandle = m_unitPS.GetConstant("tmCol");

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Control camera
	m_camera.Update(m_mouse, m_terrain, deltaTime);
	m_mouse.Update(m_terrain);

	if (KEYDOWN('W'))
	{
		m_wireframe = !m_wireframe;
		Sleep(300);
	}
	if (KEYDOWN('1'))
	{
		m_placeType = 0;
		m_placeBuilding = true;
	}
	if (KEYDOWN('2'))
	{
		m_placeType = 1;
		m_placeBuilding = true;
	}
	if (KEYDOWN('3'))
	{
		m_placeType = 2;
		m_placeBuilding = true;
	}
	else if (KEYDOWN(VK_SPACE))
	{
		m_terrain.GenerateRandomTerrain(9);

		for (int i = 0; i < m_buildings.size(); i++)
			if (m_buildings[i] != NULL)
				delete m_buildings[i];
		m_buildings.clear();
	}
	else if (KEYDOWN(VK_ESCAPE))
	{
		Quit();
	}

	//Create building to place...
	if (m_pBuildToPlace != NULL)delete m_pBuildToPlace;
	m_pBuildToPlace = NULL;

	if (m_placeBuilding)
		m_pBuildToPlace = new BUILDING(m_placeType, m_mouse.m_mappos, &m_terrain, false, m_pDevice);

	//Mouse input
	if (m_mouse.ClickLeft() && m_placeBuilding)
	{
		m_mouse.DisableInput(300);

		if (PlaceOk(m_placeType, m_mouse.m_mappos, &m_terrain))
		{
			m_buildings.push_back(new BUILDING(m_placeType, m_mouse.m_mappos, &m_terrain, true, m_pDevice));
			m_placeBuilding = false;
		}
	}
	else if (m_mouse.ClickRight() && m_placeBuilding)
		m_placeBuilding = false;

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	// Clear the viewport
	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

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
		if (m_wireframe)m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		m_terrain.Render(m_camera);

		D3DXMATRIX world;
		D3DXMatrixIdentity(&world);

		m_unitVS.SetMatrix(worldHandle, world);
		m_unitVS.SetMatrix(viewProjHandle, m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix());

		D3DXVECTOR3 sun;
		D3DXVECTOR3 vec = D3DXVECTOR3(0.5f, 1.0f, -0.5);
		D3DXVec3Normalize(&sun, &vec);
		m_unitVS.SetVector3(sunHandle, sun);
		m_unitPS.SetVector4(teamColHandle, D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f));
		m_unitVS.SetVector4(vertexCol, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));

		m_unitVS.Begin();
		m_unitPS.Begin();

		for (int i = 0; i < m_buildings.size(); i++)
			if (m_buildings[i] != NULL && !m_camera.Cull(m_buildings[i]->GetBoundingBox()))
			{
				m_unitVS.SetMatrix(worldHandle, m_buildings[i]->GetWorldMatrix());
				m_buildings[i]->Render();
			}

		//Render building to place...
		if (m_placeBuilding)
		{
			if (PlaceOk(m_placeType, m_mouse.m_mappos, &m_terrain))
				m_unitVS.SetVector4(vertexCol, D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f));
			else m_unitVS.SetVector4(vertexCol, D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f));
			m_unitVS.SetMatrix(worldHandle, m_pBuildToPlace->GetWorldMatrix());
			m_pBuildToPlace->Render();
		}

		m_unitPS.End();
		m_unitVS.End();

		//Draw selections
		for (int i = 0; i < m_buildings.size(); i++)
			if (m_buildings[i] != NULL && !m_camera.Cull(m_buildings[i]->GetBoundingBox()))
				m_buildings[i]->PaintSelected();

		Select();
		m_mouse.Paint();

		RECT r[] = { {10, 10, 0, 0}, {10, 50, 0, 0}, {10, 70, 0, 0}, {10, 90, 0, 0}, {10, 110, 0, 0}, {720, 10, 0, 0} };
		m_pFont->DrawText(NULL, "Space: Randomize Terrain", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "1: Townhall", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "2: Barracks", -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "3: Tower", -1, &r[3], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "Left Mouse Button: Place Building", -1, &r[4], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

		//FPS
		char number[50];
		std::string text = "FPS: ";
		//text += _itoa(m_lastFps, number, 10);
		_itoa_s(m_lastFps, number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[5], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

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
		UnloadMapObjectResources();
		UnloadBuildingResources();

		for (int i = 0; i < m_buildings.size(); i++)
			if (m_buildings[i] != NULL)
				delete m_buildings[i];
		m_buildings.clear();

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

int APPLICATION::GetBuilding()
{
	//Find closest unit
	int build = -1;
	float bestDist = 100000.0f;

	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	m_pDevice->SetTransform(D3DTS_WORLD, &world);
	RAY ray = m_mouse.GetRay();

	for (int i = 0; i < m_buildings.size(); i++)
	{
		float dist = ray.Intersect(m_buildings[i]->GetBoundingBox());

		if (dist >= 0.0f && dist < bestDist)
		{
			build = i;
			bestDist = dist;
		}
	}

	return build;
}

void APPLICATION::Select()
{
	try
	{
		if (m_mouse.ClickLeft())	// If the mouse button is pressed
		{
			for (int i = 0; i < m_buildings.size(); i++)	//Deselect all buildings
				m_buildings[i]->m_selected = false;

			int build = GetBuilding();

			if (build > -1)m_buildings[build]->m_selected = true;
		}
	}
	catch (...)
	{
		debug.Print("Error in APPLICATION::Select()");
	}
}