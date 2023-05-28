//////////////////////////////////////////////////////////////
// Example 14.1: Game Brains!								//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "shader.h"
#include "terrain.h"
#include "camera.h"
#include "mouse.h"
#include "player.h"
#include "effect.h"
#include "particles.h"

//Effect Pool
extern std::vector<EFFECT*> effects;

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

	void AddPlayers(int noPlayers);
	void FogOfWar();
	void UpdateMiniMap();
	void RenderMiniMap(RECT dest);

private:
	IDirect3DDevice9* m_pDevice;
	TERRAIN m_terrain;
	CAMERA m_camera;
	MOUSE m_mouse;
	std::vector<PLAYER*> m_players;

	bool m_wireframe, m_paused;
	ID3DXFont* m_pFont;
	DWORD m_time;
	int m_fps, m_lastFps;
	int m_thisPlayer;
	float m_gameSpeed;

	HWND m_mainWindow;
	ID3DXLine* m_pLine;

	//Fog-of-War variables
	IDirect3DTexture9* m_pVisibleTexture, * m_pVisitedTexture;
	SHADER m_visitedShader, m_FogOfWarShader;
	ID3DXSprite* m_pSprite;
	bool m_firstFogOfWar;

	//Minimap
	IDirect3DTexture9* m_pMiniMap;
	IDirect3DTexture9* m_pMiniMapBorder;
	SHADER m_miniMapShader;
	RECT m_miniMapRect;
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
	m_wireframe = false;
	srand(GetTickCount());
	m_fps = m_lastFps = m_thisPlayer = 0;
	m_time = GetTickCount();
	m_pLine = NULL;
	m_firstFogOfWar = true;
	m_gameSpeed = 3.0f;
	m_paused = false;

	m_pVisibleTexture = m_pVisitedTexture = m_pMiniMap = NULL;
	m_pMiniMapBorder = NULL;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 14.1: Game Brains!", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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
	LoadUnitResources(m_pDevice);
	LoadBuildingResources(m_pDevice);
	LoadPlayerResources(m_pDevice);
	LoadEffectResources(m_pDevice);
	LoadParticleResources(m_pDevice);

	m_mouse.InitMouse(m_pDevice, m_mainWindow);

	m_camera.Init(m_pDevice);
	m_camera.m_fov = 0.6f;
	m_camera.m_radius = 50.0f;

	D3DXCreateLine(m_pDevice, &m_pLine);

	//Set sampler state
	for (int i = 0; i < 8; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	//Create Fog-of-war textures
	if (FAILED(m_pDevice->CreateTexture(256, 256, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pVisibleTexture, NULL)))debug.Print("Failed to create texture: m_pVisibleTexture");
	if (FAILED(m_pDevice->CreateTexture(256, 256, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pVisitedTexture, NULL)))debug.Print("Failed to create texture: m_pVisitedTexture");

	//Fog-of-war Shaders
	m_visitedShader.Init(m_pDevice, "shaders/visited.ps", PIXEL_SHADER);
	m_FogOfWarShader.Init(m_pDevice, "shaders/FogOfWar.ps", PIXEL_SHADER);
	m_firstFogOfWar = true;

	//Create Minimap
	if (FAILED(m_pDevice->CreateTexture(256, 256, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pMiniMap, NULL)))debug.Print("Failed to create texture: m_pMinimap");
	if (FAILED(D3DXCreateTextureFromFile(m_pDevice, "textures/minimap.dds", &m_pMiniMapBorder)))debug.Print("Could not load minimap.dds");
	m_miniMapShader.Init(m_pDevice, "shaders/minimap.ps", PIXEL_SHADER);
	RECT r = { 611, 9, 791, 189 };
	m_miniMapRect = r;

	//Sprite 
	D3DXCreateSprite(m_pDevice, &m_pSprite);

	//Init Terrain
	m_terrain.Init(m_pDevice, INTPOINT(150, 150));

	//Add Players
	AddPlayers(4);

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Remove dead effects
	std::vector<EFFECT*>::iterator i;
	std::vector<EFFECT*> todelete;
	for (i = effects.begin(); i != effects.end();i++)
	{
		if ((*i)->isDead())
		{
			//delete (*i);
			//effects.erase(i);
			todelete.push_back(*i);
		}
		else
		{
			(*i)->Update(deltaTime * m_gameSpeed);
			//i++;
		}
	}
	for (auto& effect : todelete) {
		effects.erase(std::find(effects.begin(), effects.end(), effect));
		delete effect;
	}

	//Pause game
	if (m_paused && m_gameSpeed > 0.0f)
	{
		m_gameSpeed -= deltaTime * 2.0f;
		if (m_gameSpeed < 0.0f)m_gameSpeed = 0.0f;
	}
	else if (!m_paused && m_gameSpeed < 3.0f)
	{
		m_gameSpeed += deltaTime * 2.0f;
		if (m_gameSpeed > 3.0f)m_gameSpeed = 3.0f;
	}


	//Update Players
	for (int i = 0; i < m_players.size(); i++)
		if (m_players[i] != NULL)
			m_players[i]->UpdateMapObjects(deltaTime * m_gameSpeed);

	//Update Fog-of-War
	FogOfWar();

	//Control camera
	m_camera.Update(m_mouse, m_terrain, deltaTime);
	m_mouse.Update(&m_terrain);

	//Update SightMatrices & visible variables
	if (m_terrain.m_updateSight)
	{
		m_terrain.m_updateSight = false;

		if (m_thisPlayer < m_players.size() && m_players[m_thisPlayer] != NULL)
			m_terrain.UpdateSightMatrices(m_players[m_thisPlayer]->m_mapObjects);

		for (int i = 0; i < m_players.size(); i++)
			if (m_players[i] != NULL)
				m_players[i]->IsMapObjectsVisible();

		//Update Minimap		
		UpdateMiniMap();
	}

	//Keyboard input
	if (KEYDOWN('W'))
	{
		//m_wireframe = !m_wireframe;
		//Sleep(300);
	}
	else if (KEYDOWN(VK_SPACE))
	{
		if (m_gameSpeed == 0.0f || m_gameSpeed == 3.0f)
			m_paused = !m_paused;
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
	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

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

		//Render terrain
		m_terrain.Render(m_camera);

		//Render units and buildings
		for (int i = 0; i < m_players.size(); i++)
			if (m_players[i] != NULL)
				m_players[i]->RenderMapObjects(m_camera);

		//Render Effects
		for (int i = 0; i < effects.size(); i++)
			if (effects[i] != NULL)
				effects[i]->Render();

		//Render Minimap
		RenderMiniMap(m_miniMapRect);

		RECT r[] = { {10, 580, 0, 0}, {520, 10, 0, 0}, {10, 10, 0, 0}, {5, 20, 0, 0} };
		m_pFont->DrawText(NULL, "Space: Pause Game", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		//FPS
		char number[50];
		std::string text = "FPS: ";
		//text += _itoa(m_lastFps, number, 10);
		_itoa_s(m_lastFps, number, sizeof(number), 10);
		text += number;
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		m_pFont->DrawText(NULL, "Kills:", -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_pFont->DrawText(NULL, "-------------", -1, &r[3], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		//Team Kill Score
		for (int i = 0; i < m_players.size(); i++)
		{
			if (i == 0)text = "RED: ";
			if (i == 1)text = "GREEN: ";
			if (i == 2)text = "BLUE: ";
			if (i == 3)text = "YELLOW: ";

			//text += itoa(m_players[i]->m_numKills, number, 10);
			_itoa_s(m_players[i]->m_numKills, number, sizeof(number), 10);
			text == number;
			RECT rc = { 10, 35 + i * 20, 0, 0 };
			m_pFont->DrawText(NULL, text.c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		}

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
		m_terrain.Release();

		UnloadObjectResources();
		UnloadMapObjectResources();
		UnloadUnitResources();
		UnloadBuildingResources();
		UnloadPlayerResources();
		UnloadEffectResources();
		UnloadParticleResources();

		if (m_pVisibleTexture)m_pVisibleTexture->Release();
		if (m_pVisitedTexture)m_pVisitedTexture->Release();

		if (m_pMiniMap)m_pMiniMap->Release();
		if (m_pMiniMapBorder)m_pMiniMapBorder->Release();

		for (int i = 0; i < m_players.size(); i++)
			if (m_players[i] != NULL)
				delete m_players[i];
		m_players.clear();

		m_pFont->Release();
		m_pSprite->Release();
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

void APPLICATION::AddPlayers(int noPlayers)
{
	m_thisPlayer = 0;

	for (int i = 0; i < m_players.size(); i++)
		if (m_players[i] != NULL)
			delete m_players[i];
	m_players.clear();

	INTPOINT startLocations[] = { INTPOINT(30,30), INTPOINT(120,30), INTPOINT(30,120), INTPOINT(120,120) };
	//INTPOINT startLocations[] = {INTPOINT(15,15), INTPOINT(50,50), INTPOINT(30,120), INTPOINT(120,120)};

	D3DXVECTOR4 teamCols[] = { D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f), D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f),
							  D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f) };

	int controllers[] = { COMPUTER, COMPUTER, COMPUTER, COMPUTER };

	if (noPlayers < 2)noPlayers = 2;
	if (noPlayers > 4)noPlayers = 4;

	for (int i = 0; i < noPlayers; i++)
	{
		m_terrain.Progress("Creating Players", i / (float)noPlayers);
		m_players.push_back(new PLAYER(i, controllers[i], teamCols[i], startLocations[i], &m_terrain, m_pDevice));
	}

	//Center camera focus on the team...
	m_camera.m_focus = m_terrain.GetWorldPos(m_players[m_thisPlayer]->GetCenter());
}

void APPLICATION::FogOfWar()
{
	try
	{
		if (m_pVisibleTexture == NULL || m_pVisitedTexture == NULL)return;

		//Set orthogonal rendering view & projection
		m_terrain.SetOrthogonalView();

		//Retrieve the surface of the back buffer
		IDirect3DSurface9* backSurface = NULL;
		m_pDevice->GetRenderTarget(0, &backSurface);

		//Render the Visible Texture here...
		{
			//Set texture stages and Renderstates
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);

			m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);

			//Get the surface of the m_pVisibleTexture
			IDirect3DSurface9* visibleSurface = NULL;
			m_pVisibleTexture->GetSurfaceLevel(0, &visibleSurface);

			//Set render target to the visible surface
			m_pDevice->SetRenderTarget(0, visibleSurface);

			//Clear render target to black
			m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
			//m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

			m_pDevice->BeginScene();

			//Render the sightTexture for all map objects in thisPlayer.
			if (m_thisPlayer < m_players.size() && m_players[m_thisPlayer] != NULL)
				for (int u = 0; u < m_players[m_thisPlayer]->m_mapObjects.size(); u++)
					if (m_players[m_thisPlayer]->m_mapObjects[u] != NULL)
						m_players[m_thisPlayer]->m_mapObjects[u]->RenderSightMesh();

			m_pDevice->EndScene();

			//Restore renderstates etc.
			m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

			//Release visible surface
			visibleSurface->Release();
		}

		//Render the Visited Texture here...
		{
			IDirect3DSurface9* visitedSurface = NULL;
			m_pVisitedTexture->GetSurfaceLevel(0, &visitedSurface);

			//Render to the visted texture
			m_pDevice->SetRenderTarget(0, visitedSurface);
			if (m_firstFogOfWar)
			{
				//m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
				m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x88888888, 1.0f, 0);
				m_firstFogOfWar = false;
			}

			m_pDevice->BeginScene();
			m_pDevice->SetTexture(0, m_pVisibleTexture);
			m_pDevice->SetTexture(1, m_pVisitedTexture);

			m_pSprite->Begin(0);
			m_visitedShader.Begin();
			D3DXVECTOR3 vec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			m_pSprite->Draw(m_pVisibleTexture, NULL, NULL, &vec, 0xffffffff);
			m_pSprite->End();
			m_visitedShader.End();

			m_pDevice->EndScene();

			//Release visited surface
			visitedSurface->Release();
		}

		//Render the final FogOfWarTexture
		{
			//Get and set surface of the FogOfWarTexture...
			IDirect3DSurface9* FogOfWarSurface = NULL;
			m_terrain.m_pFogOfWarTexture->GetSurfaceLevel(0, &FogOfWarSurface);
			m_pDevice->SetRenderTarget(0, FogOfWarSurface);

			m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
			m_pDevice->BeginScene();

			//Set Textures
			m_pDevice->SetTexture(0, m_pVisibleTexture);
			m_pDevice->SetTexture(1, m_pVisitedTexture);
			m_pDevice->SetTexture(2, m_terrain.m_pLightMap);

			//Draw to the Fog-of-War texture
			m_pSprite->Begin(0);
			m_FogOfWarShader.Begin();
			D3DXVECTOR3 vec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			m_pSprite->Draw(m_pVisibleTexture, NULL, NULL, &vec, 0xffffffff);
			m_pSprite->End();
			m_FogOfWarShader.End();

			m_pDevice->EndScene();

			//Release fog of war surface
			FogOfWarSurface->Release();
		}

		//Reset render target to back buffer
		m_pDevice->SetRenderTarget(0, backSurface);
		backSurface->Release();
	}
	catch (...)
	{
		debug.Print("Error in APPLICATION::FogOfWar()");
	}
}

void APPLICATION::UpdateMiniMap()
{
	//Retrieve the surface of the back buffer
	IDirect3DSurface9* backSurface = NULL;
	m_pDevice->GetRenderTarget(0, &backSurface);

	//Get and set surface of the FogOfWarTexture...
	IDirect3DSurface9* minimapSurface = NULL;
	m_pMiniMap->GetSurfaceLevel(0, &minimapSurface);
	m_pDevice->SetRenderTarget(0, minimapSurface);

	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
	m_pDevice->BeginScene();

	//Set Textures
	m_pDevice->SetTexture(0, m_terrain.m_pLandScape);
	m_pDevice->SetTexture(1, m_terrain.m_pFogOfWarTexture);

	//Draw to the minimap texture
	m_pSprite->Begin(0);
	m_miniMapShader.Begin();
	D3DXVECTOR3 vec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_pSprite->Draw(m_terrain.m_pLandScape, NULL, NULL, &vec, 0xffffffff);
	m_pSprite->End();
	m_miniMapShader.End();

	m_pDevice->EndScene();

	//Draw units and buildings in the player team color
	for (int p = 0; p < m_players.size(); p++)
		if (m_players[p] != NULL)
		{
			std::vector<D3DRECT> rects;

			//Get rectangles in "Minimap Space"
			for (int m = 0; m < m_players[p]->m_mapObjects.size(); m++)
				if (m_players[p]->m_mapObjects[m] != NULL && !m_players[p]->m_mapObjects[m]->m_dead)
					if (!m_players[p]->m_mapObjects[m]->m_isBuilding)
					{
						INTPOINT mappos = m_players[p]->m_mapObjects[m]->m_mappos;

						//Only add units standing on visible tiles
						if (m_terrain.m_pVisibleTiles[mappos.x + mappos.y * m_terrain.m_size.x] || 1)
						{
							INTPOINT pos(256.0f * (mappos.x / (float)m_terrain.m_size.x),
								256.0f * (mappos.y / (float)m_terrain.m_size.y));

							rects.push_back(SetRect(pos.x - 1, pos.y - 1,
								pos.x + 2, pos.y + 2));
						}
					}
					else
					{
						RECT r = m_players[p]->m_mapObjects[m]->GetMapRect(0);

						//Add only those parts of the buildings standing on visited tiles
						for (int y = r.top; y <= r.bottom; y++)
							for (int x = r.left; x <= r.right; x++)
								if (m_terrain.m_pVisitedTiles[x + y * m_terrain.m_size.x] || 1)
								{
									INTPOINT pos(256.0f * (x / (float)m_terrain.m_size.x),
										256.0f * (y / (float)m_terrain.m_size.y));

									rects.push_back(SetRect(pos.x - 1, pos.y - 1,
										pos.x + 2, pos.y + 2));
								}
					}

			//Clear rectangles using the team color
			if (!rects.empty())
			{
				D3DXCOLOR c = D3DCOLOR_XRGB((int)(m_players[p]->m_teamColor.x * 255),
					(int)(m_players[p]->m_teamColor.y * 255),
					(int)(m_players[p]->m_teamColor.z * 255));

				m_pDevice->Clear(rects.size(), &rects[0], D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, c, 1.0f, 0);
			}
		}

	//Reset render target to back buffer
	m_pDevice->SetRenderTarget(0, backSurface);

	//Release surfaces
	backSurface->Release();
	minimapSurface->Release();
}

void APPLICATION::RenderMiniMap(RECT dest)
{
	float width = dest.right - dest.left;
	float height = dest.bottom - dest.top;

	D3DXVECTOR2 scale = D3DXVECTOR2(width / 256.0f,
		height / 256.0f);
	D3DXMATRIX sca;
	D3DXMatrixScaling(&sca, scale.x, scale.y, 1.0f);
	m_pSprite->SetTransform(&sca);

	m_pSprite->Begin(0);
	D3DXVECTOR3 vec = D3DXVECTOR3(dest.left / scale.x, dest.top / scale.y, 0.0f);
	m_pSprite->Draw(m_pMiniMap, NULL, NULL, &vec, 0xffffffff);
	m_pSprite->End();

	D3DXMatrixIdentity(&sca);
	m_pSprite->SetTransform(&sca);

	//Move camera using minimap
	if (m_mouse.PressInRect(dest))
	{
		int x = ((m_mouse.x - dest.left) / width) * m_terrain.m_size.x;
		int y = ((m_mouse.y - dest.top) / height) * m_terrain.m_size.y;

		m_camera.m_focus = m_terrain.GetWorldPos(INTPOINT(x, y));
	}

	//Calculate m_camera frustum viewpoints
	D3DXMATRIX view, proj, viewInverse;

	view = m_camera.GetViewMatrix();
	proj = m_camera.GetProjectionMatrix();

	//fov_x & fov_y Determines the size of the frustum representation
	float screenRatio = proj(0, 0) / proj(1, 1);
	float fov_x = 0.4f;
	float fov_y = fov_x * screenRatio;

	//Initialize the four rays
	D3DXVECTOR3 org = D3DXVECTOR3(0.0f, 0.0f, 0.0f);	//Same origin

	//Four different directions
	D3DXVECTOR3 dir[4] = { D3DXVECTOR3(-fov_x, fov_y, 1.0f),
						  D3DXVECTOR3(fov_x, fov_y, 1.0f),
						  D3DXVECTOR3(fov_x, -fov_y, 1.0f),
						  D3DXVECTOR3(-fov_x, -fov_y, 1.0f) };

	//Our resulting minimap coordinates
	D3DXVECTOR2 points[5];

	//View matrix inverse
	D3DXMatrixInverse(&viewInverse, 0, &view);
	D3DXVec3TransformCoord(&org, &org, &viewInverse);

	//Ground plane
	D3DXPLANE plane;
	D3DXVECTOR3 point = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXPlaneFromPointNormal(&plane, &point, &normal);

	bool ok = true;

	//check where each ray intersects with the ground plane
	for (int i = 0; i < 4 && ok; i++)
	{
		//Transform ray direction
		D3DXVec3TransformNormal(&dir[i], &dir[i], &viewInverse);
		D3DXVec3Normalize(&dir[i], &dir[i]);
		dir[i] *= 1000.0f;

		//Find intersection point
		D3DXVECTOR3 hit;
		if (D3DXPlaneIntersectLine(&hit, &plane, &org, &dir[i]) == NULL)
			ok = false;

		//Make sure the intersection point is on the positive side of the near plane
		D3DXPLANE n = m_camera.m_frustum[4];
		float distance = n.a * hit.x + n.b * hit.y + n.c * hit.z + n.d;
		if (distance < 0.0f)ok = false;

		//Convert the intersection point to a minimap coordinate
		if (ok)
		{
			points[i].x = (hit.x / (float)m_terrain.m_size.x) * width;
			points[i].y = (-hit.z / (float)m_terrain.m_size.y) * height;
		}
	}

	//Set the end point to equal the starting point
	points[4] = points[0];

	//Set viewport to destination rectangle only...
	D3DVIEWPORT9 v1, v2;

	v1.X = dest.left;
	v1.Y = dest.top;
	v1.Width = width;
	v1.Height = height;
	v1.MinZ = 0.0f;
	v1.MaxZ = 1.0f;

	m_pDevice->GetViewport(&v2);
	m_pDevice->SetViewport(&v1);

	//Draw camera frustum in the minimap
	if (ok)
	{
		m_pLine->SetWidth(1.0f);
		m_pLine->SetAntialias(true);
		m_pLine->Begin();
		m_pLine->Draw(&points[0], 5, 0x44ffffff);
		m_pLine->End();
	}

	//Reset viewport
	m_pDevice->SetViewport(&v2);

	//Draw minimap border
	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	vec = D3DXVECTOR3(v2.Width - 256, 0.0f, 0.0f);
	m_pSprite->Draw(m_pMiniMapBorder, NULL, NULL, &vec, 0xffffffff);
	m_pSprite->End();
}