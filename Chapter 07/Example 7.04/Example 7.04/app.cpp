//////////////////////////////////////////////////////////////
// Example 7.4: Placing a Weapon in the hand				//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "skinnedMesh.h"
#include "shader.h"
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
	DWORD FtoDword(float f) { return *((DWORD*)&f); }

private:
	IDirect3DDevice9* m_pDevice;
	SKINNEDMESH m_skinnedMesh;
	BONE* m_pHand;
	MESH m_weapons[4];
	int m_activeWeapon;

	float m_angle, m_time;
	D3DXVECTOR3 m_handOffset;
	std::vector<std::string> m_animations;
	int m_activeAnimation;
	D3DLIGHT9 m_light;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;

	//Shaders
	SHADER m_unitVS, m_unitPS;
	D3DXHANDLE m_worldHandle, m_viewProjHandle, m_sunHandle;
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
	srand(GetTickCount());
	m_angle = m_time = 0.0f;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 7.4: Placing a Weapon in the hand", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	// Create m_light
	::ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = D3DLIGHT_DIRECTIONAL;
	m_light.Ambient = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Diffuse = D3DXCOLOR(0.9, 0.9, 0.9, 1.0f);
	m_light.Specular = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

	//Load m_weapons
	m_activeWeapon = 0;
	m_weapons[0].Load("mesh/club.x", m_pDevice);
	m_weapons[1].Load("mesh/sword.x", m_pDevice);
	m_weapons[2].Load("mesh/axe.x", m_pDevice);
	m_weapons[3].Load("mesh/flowers.x", m_pDevice);

	//Skinned mesh
	m_skinnedMesh.Load("mesh/drone.x", m_pDevice);
	m_animations = m_skinnedMesh.GetAnimations();
	m_activeAnimation = m_animations.size() - 3;
	m_skinnedMesh.SetAnimation((char*)m_animations[m_activeAnimation].c_str());

	//Find left hand bone
	m_pHand = m_skinnedMesh.FindBone("Bone19");
	m_handOffset = D3DXVECTOR3(0.5f, 0.0f, -0.07f);

	//Set sampler state
	for (int i = 0; i < 8; i++)
	{
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

	//Setup shaders
	m_unitVS.Init(m_pDevice, "shaders/lighting.vs", VERTEX_SHADER);
	m_worldHandle = m_unitVS.GetConstant("matW");
	m_viewProjHandle = m_unitVS.GetConstant("matVP");
	m_sunHandle = m_unitVS.GetConstant("DirToSun");

	m_unitPS.Init(m_pDevice, "shaders/unit.ps", PIXEL_SHADER);

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	m_angle += deltaTime * 0.5f;
	if (m_angle > D3DX_PI * 2.0f)
		m_angle -= D3DX_PI * 2.0f;

	m_time = deltaTime * 0.5f;

	if (KEYDOWN(VK_SPACE))
	{
		m_activeWeapon++;
		if (m_activeWeapon >= 4)m_activeWeapon = 0;
		Sleep(300);
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
	m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);

	//Set camera
	D3DXMATRIX view, proj, world;
	D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 10.0f, -50.0f);
	D3DXVECTOR3 center = D3DXVECTOR3(0.0f, 4.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&view, &eye, &center, &up);
	D3DXMatrixOrthoLH(&proj, 10.0f, 9.0f, 0.1f, 1000.0f);

	//Set Skeleton to rotate around the Y-axis
	D3DXMATRIX r, s;
	D3DXMatrixRotationYawPitchRoll(&r, m_angle, 0.0f, 0.0f);
	D3DXMatrixScaling(&s, 1.2f, 1.2f, 1.2f);
	world = s * r;

	m_pDevice->SetTransform(D3DTS_VIEW, &view);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);
	m_pDevice->SetTransform(D3DTS_WORLD, &world);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_unitVS.SetMatrix(m_worldHandle, world);
		m_unitVS.SetMatrix(m_viewProjHandle, view * proj);

		D3DXVECTOR3 sun;
		D3DXVECTOR3 vec = D3DXVECTOR3(0.5f, 1.0f, -0.2);
		D3DXVec3Normalize(&sun, &vec);
		m_unitVS.SetVector3(m_sunHandle, sun);

		m_unitVS.Begin();
		m_unitPS.Begin();

		m_skinnedMesh.SetPose(world, NULL, m_time);
		m_skinnedMesh.Render(NULL);

		m_unitPS.End();
		m_unitVS.End();

		//Render weapon
		if (m_pHand != NULL)
		{
			D3DXMATRIX boneMatrix = m_pHand->CombinedTransformationMatrix;
			D3DXMATRIX offset, weaponTransform;
			D3DXMatrixTranslation(&offset, m_handOffset.x, m_handOffset.y, m_handOffset.z);

			weaponTransform = offset * boneMatrix * world;

			m_pDevice->SetTransform(D3DTS_WORLD, &weaponTransform);
			m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
			m_weapons[m_activeWeapon].Render();
		}

		RECT r = { 10, 10, 0, 0 };
		m_pFont->DrawText(NULL, "Space: Change Weapon", -1, &r, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);

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