//////////////////////////////////////////////////////////////
// Example 5.8: Progressive Meshes							//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include "debug.h"
#include "mesh.h"

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
	MESH* m_pFarmerMesh;
	MESHINSTANCE m_farmer1, m_farmer2;

	D3DLIGHT9 m_light;
	bool m_wireframe;
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
	m_pFarmerMesh = NULL;
	m_pFont = NULL;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 5.8: Progressive Meshes", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	//Load mech
	m_pFarmerMesh = new MESH("objects/farmer.x", m_pDevice);

	m_farmer1.SetMesh(m_pFarmerMesh);
	m_farmer2.SetMesh(m_pFarmerMesh);
	m_farmer1.SetPosition(D3DXVECTOR3(-2.5f, 0.0f, 0.0f));
	m_farmer2.SetPosition(D3DXVECTOR3(2.5f, 0.0f, 0.0f));

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

	//Rotate farmers
	m_farmer1.SetRotation(m_farmer1.m_rot + D3DXVECTOR3(0.0f, deltaTime * 0.5f, 0.0f));
	if (m_farmer1.m_rot.y > D3DX_PI * 2.0f)m_farmer1.m_rot.y -= D3DX_PI * 2.0f;

	m_farmer2.SetRotation(m_farmer2.m_rot + D3DXVECTOR3(0.0f, deltaTime * 0.5f, 0.0f));
	if (m_farmer2.m_rot.y > D3DX_PI * 2.0f)m_farmer2.m_rot.y -= D3DX_PI * 2.0f;

	if (KEYDOWN(VK_DOWN))
	{
		m_farmer2.SetLOD(m_farmer2.GetNumProgressiveFaces() - 5);
		Sleep(30);
	}
	else if (KEYDOWN(VK_UP))
	{
		m_farmer2.SetLOD(m_farmer2.GetNumProgressiveFaces() + 5);
		Sleep(30);
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
		//Set camera
		D3DXMATRIX view, proj;
		D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 10.0f, -50.0f);
		D3DXVECTOR3 at = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&view, &eye, &at, &up);
		D3DXMatrixOrthoLH(&proj, 10.0f, 9.0f, 0.1f, 1000.0f);
		m_pDevice->SetTransform(D3DTS_VIEW, &view);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

		if (m_wireframe)m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		m_farmer1.Render();
		m_farmer2.RenderProgressive();

		//Number of polygons
		char number[50];
		//std::string text = _itoa(m_farmer2.GetNumProgressiveFaces(), number, 10);
		std::string text;
		_itoa_s(m_farmer2.GetNumProgressiveFaces(), number, sizeof(number), 10);
		text += number;
		text += " polygons (UP/DOWN Arrow)";

		RECT r[] = { {170, 520, 0, 0}, {530, 520, 0, 0}, {470, 540, 0, 0}, {130, 540, 0, 0} };
		m_pFont->DrawText(NULL, "Original", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "Progressive Mesh", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, text.c_str(), -1, &r[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
		m_pFont->DrawText(NULL, "(W)ireframe On/Off", -1, &r[3], DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

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

		if (m_pFarmerMesh != NULL)
			m_pFarmerMesh->Release();

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