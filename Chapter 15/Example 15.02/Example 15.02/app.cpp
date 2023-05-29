#include "app.h"

APPLICATION app;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	if (FAILED(app.Init(hInstance, 800, 600, true)))
		return 0;

	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	int startTime = timeGetTime();

	while (msg.message != WM_QUIT)
	{
		try
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
		catch (...) {}
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
	m_mainWindow = CreateWindow("D3DWND", "Example 15.2: RTS Network Framework", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	LoadObjectResources(m_pDevice);
	LoadMapObjectResources(m_pDevice);
	LoadUnitResources(m_pDevice);
	LoadBuildingResources(m_pDevice);
	LoadPlayerResources(m_pDevice);

	m_thisPlayer = 0;
	m_terrain.Init(m_pDevice, INTPOINT(150, 150));
	m_camera.Init(m_pDevice);
	m_camera.m_focus = m_terrain.GetWorldPos(INTPOINT(75, 75));

	m_mouse.InitMouse(m_pDevice, m_mainWindow);
	m_lobby.Init(m_pDevice);

	//Set sampler state
	for (int i = 0; i < 8; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	try
	{
		//Control camera
		m_mouse.Update(&m_terrain);

		if (!m_lobby.Done())
			m_lobby.Update(deltaTime, m_mouse);
		else
		{
			m_camera.Update(m_mouse, m_terrain, deltaTime);

			//Update Players
			for (int i = 0; i < m_players.size(); i++)
				if (m_players[i] != NULL)
					m_players[i]->UpdateMapObjects(deltaTime);

			//Order units of team 0 around...
			if (m_thisPlayer < m_players.size() && m_players[m_thisPlayer] != NULL)
				m_players[m_thisPlayer]->UnitOrders(m_mouse);
		}

		//Keyboard input
		if (KEYDOWN('W'))
		{
		}
		else if (KEYDOWN(VK_ESCAPE))
		{
			Quit();
		}
	}
	catch (...) {}

	return S_OK;
}

HRESULT APPLICATION::Render()
{
	try
	{
		// Clear the viewport
		m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

		// Begin the scene 
		if (SUCCEEDED(m_pDevice->BeginScene()))
		{
			if (!m_lobby.Done())
				m_lobby.Draw(m_mouse);
			else
			{
				m_terrain.Render(m_camera);

				for (int i = 0; i < m_players.size(); i++)
					if (m_players[i] != NULL)
						m_players[i]->RenderMapObjects(m_camera);

				//Select units
				if (m_thisPlayer < m_players.size() && m_players[m_thisPlayer] != NULL)
				{
					m_players[m_thisPlayer]->PaintSelectedMapObjects(m_camera);
					m_players[m_thisPlayer]->Select(m_mouse);
				}
			}

			m_mouse.Paint();

			// End the scene.
			m_pDevice->EndScene();
			m_pDevice->Present(0, 0, 0, 0);
		}
	}
	catch (...) {}

	return S_OK;
}

HRESULT APPLICATION::Cleanup()
{
	try
	{
		m_terrain.Release();

		UnloadObjectResources();
		UnloadMapObjectResources();
		UnloadUnitResources();
		UnloadBuildingResources();
		UnloadPlayerResources();

		for (int i = 0; i < m_players.size(); i++)
			if (m_players[i] != NULL)
				delete m_players[i];
		m_players.clear();

		m_pDevice->Release();

		debug.Print("Application terminated");
	}
	catch (...) {}

	return S_OK;
}

HRESULT APPLICATION::Quit()
{
	try
	{
		::DestroyWindow(m_mainWindow);
		::PostQuitMessage(0);
	}
	catch (...) {}

	return S_OK;
}

void APPLICATION::AddPlayers(int num, int activePlayer)
{
	srand(num);

	m_thisPlayer = activePlayer;

	for (int i = 0; i < m_players.size(); i++)
		if (m_players[i] != NULL)
			delete m_players[i];
	m_players.clear();

	INTPOINT startLocations[] = { INTPOINT(30,30), INTPOINT(120,30), INTPOINT(30,120), INTPOINT(120,120) };
	D3DXVECTOR4 teamCols[] = { D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f), D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f),
							  D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f) };

	if (num < 2)num = 2;
	if (num > 4)num = 4;

	for (int i = 0; i < num; i++)
	{
		m_terrain.Progress("Creating Players", i / (float)num);
		m_players.push_back(new PLAYER(i, teamCols[i], startLocations[i], &m_terrain, m_pDevice));
	}

	//Center camera focus on the team...
	m_camera.m_focus = m_terrain.GetWorldPos(m_players[m_thisPlayer]->GetCenter());
}

void APPLICATION::HandleNetworkOrder(RTS_MSG* msg)
{
	if (msg == NULL)return;

	if (msg->type == GAME_MSG_ORDER)
	{
		MSG_ORDER* order = (MSG_ORDER*)msg;

		if (m_terrain.Within(order->dest))
			if (order->player >= 0 && order->player < m_players.size())
				if (order->unitID >= 0 && order->unitID < m_players[order->player]->m_mapObjects.size())
				{
					UNIT* unit = (UNIT*)m_players[order->player]->m_mapObjects[order->unitID];
					unit->MoveTo(order->dest, false, true);
				}
	}
}