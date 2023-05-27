//////////////////////////////////////////////////////////////
// Example 4.10: Mesh Instances								//
// Written by: C. Granberg, 2005							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"

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

	void CreateMeshInstances();

private:
	IDirect3DDevice9* m_pDevice;
	MESH m_mesh1, m_mesh2;
	std::vector<MESHINSTANCE> m_meshInstances;

	float m_angle;
	bool m_wireframe;
	D3DLIGHT9 m_light;
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
	m_angle = 0.0f;
	m_wireframe = false;

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
	m_mainWindow = CreateWindow("D3DWND", "Example 4.10: Mesh Instances", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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
	m_light.Direction = D3DXVECTOR3(0.7f, -0.3f, 0.0f);
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

	//Sampler states
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	//Load Meshes
	if (FAILED(m_mesh1.Load("meshes/tree.x", m_pDevice)) || FAILED(m_mesh2.Load("meshes/stone.x", m_pDevice)))
	{
		debug.Print("Failed to load meshes");
		Quit();
	}

	//Create many instances of the two meshes
	CreateMeshInstances();


	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Control camera
	m_angle += deltaTime * 0.5f;
	D3DXMATRIX  matWorld, matView, matProj;
	D3DXVECTOR3 Eye = D3DXVECTOR3(cos(m_angle) * 8.0f, 5.5f, sin(m_angle) * 8.0f);
	D3DXVECTOR3 Lookat = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	D3DXMatrixIdentity(&matWorld);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &Eye, &Lookat, &up);
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.3333f, 1.0f, 1000.0f);

	m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	if (KEYDOWN(VK_SPACE))
	{
		CreateMeshInstances();
		Sleep(300);
	}
	else if (KEYDOWN('W'))
	{
		m_wireframe = !m_wireframe;
		Sleep(300);
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

		//Render each mesh instance
		for (int i = 0; i < m_meshInstances.size(); i++)
			m_meshInstances[i].Render();

		RECT r[] = { {10, 10, 0, 0}, {10, 30, 0, 0} };
		m_pFont->DrawText(NULL, "Space: Randomize Objects", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "W: Toggle Wireframe", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

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
		m_mesh1.Release();
		m_mesh2.Release();

		m_meshInstances.clear();

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

void APPLICATION::CreateMeshInstances()
{
	m_meshInstances.clear();

	//Create a new instance with random type, rotation & scale
	for (int y = -5; y <= 5; y++)
		for (int x = -5; x <= 5; x++)
		{
			MESHINSTANCE m;

			if (rand() % 2 == 0)m.SetMesh(&m_mesh1);
			else m.SetMesh(&m_mesh2);

			m.SetPosition(D3DXVECTOR3(x, 0, y));

			float rX = D3DX_PI * ((rand() % 50) / 1000.0f);
			float rY = D3DX_PI * ((rand() % 1000) / 1000.0f);
			float rZ = D3DX_PI * ((rand() % 50) / 1000.0f);
			m.SetRotation(D3DXVECTOR3(rX, rY, rZ));

			float scale = (rand() % 750 + 250) / 1000.0f;
			m.SetScale(D3DXVECTOR3(scale, scale, scale));

			m_meshInstances.push_back(m);
		}
}