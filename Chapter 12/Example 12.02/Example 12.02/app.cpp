//////////////////////////////////////////////////////////////
// Example 12.2: Fireball!									//
// Written by: C. Granberg, 2006							//
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "skinnedmesh.h"
#include "shader.h"
#include "effect.h"
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
	void SetAnimation(const char name[]);

private:
	IDirect3DDevice9* m_pDevice;
	D3DLIGHT9 m_light;
	HWND m_mainWindow;
	ID3DXFont* m_pFont;

	//Meshes
	MESH m_target;
	MESHINSTANCE m_targetInstance;
	TRANSFORM m_magicianTrans;
	SKINNEDMESH m_skinnedMesh;
	ID3DXAnimationController* m_pAnimControl;
	bool m_attack;
	float m_attackTime, m_deltaTime;

	//Shaders
	SHADER m_unitVS, m_unitPS;
	D3DXHANDLE m_worldHandle, m_viewProjHandle, m_sunHandle, m_teamColHandle;

	//Effects 
	std::vector<EFFECT*> m_effects;
};

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
	srand(GetTickCount());
	m_attack = false;
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
	m_mainWindow = CreateWindow("D3DWND", "Example 12.2: Fireball!", WS_EX_TOPMOST, 0, 0, width, height, 0, 0, hInstance, 0);
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

	// Create m_light
	::ZeroMemory(&m_light, sizeof(m_light));
	m_light.Type = D3DLIGHT_DIRECTIONAL;
	m_light.Ambient = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Diffuse = D3DXCOLOR(0.9, 0.9, 0.9, 1.0f);
	m_light.Specular = D3DXCOLOR(0.5, 0.5, 0.5, 1.0f);
	m_light.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	m_pDevice->SetLight(0, &m_light);
	m_pDevice->LightEnable(0, true);

	LoadEffectResources(m_pDevice);

	D3DXCreateFont(m_pDevice, 18, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont);

	m_skinnedMesh.Load("units/magician.x", m_pDevice);
	m_pAnimControl = m_skinnedMesh.GetAnimationControl();
	SetAnimation("Still");

	m_target.Load("meshes/target.x", m_pDevice);
	m_targetInstance.SetMesh(&m_target);

	//Position objects
	m_targetInstance.SetPosition(D3DXVECTOR3(15.0f, 0.0f, 40.0f));
	m_targetInstance.SetRotation(D3DXVECTOR3(0.0f, 0.6f, 0.0f));
	m_targetInstance.SetScale(D3DXVECTOR3(0.7f, 0.7f, 0.7f));

	m_magicianTrans.Init(D3DXVECTOR3(-5.0f, 0.0f, -10.0f), D3DXVECTOR3(0.0f, D3DX_PI + 0.4f, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f));

	//Set sampler state
	for (int i = 0; i < 4; i++)
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

	m_unitPS.Init(m_pDevice, "shaders/teamCol.ps", PIXEL_SHADER);
	m_teamColHandle = m_unitPS.GetConstant("tmCol");

	return S_OK;
}

HRESULT APPLICATION::Update(float deltaTime)
{
	//Remove dead effects
	std::vector<EFFECT*>::iterator i;
	std::vector<EFFECT*>todelete;
	
	for (i = m_effects.begin(); i != m_effects.end();i++)
	{
		if ((*i)->isDead())
		{
			todelete.push_back(*i);
			//delete (*i);
			//m_effects.erase(i);
		}
		else
		{
			(*i)->Update(deltaTime);
			//i++;
		}
	
	}
	for (auto& effect : todelete) {
		m_effects.erase(std::find(m_effects.begin(),m_effects.end(),effect));
		delete effect;
	}
	//Control attack animation
	if (m_attack)
	{
		m_attackTime += deltaTime * 0.3f;
		m_deltaTime = deltaTime * 0.3f;
		if (m_attackTime >= 0.40f)
		{
			SetAnimation("Still");
			m_attack = false;
			m_attackTime = 0.0f;
		}
	}
	else m_deltaTime = 0.0f;

	//Keayboard input
	if (KEYDOWN(VK_SPACE))
	{
		if (m_effects.empty())
		{
			m_effects.push_back(new EFFECT_FIREBALL(m_pDevice, m_skinnedMesh.FindBone("Staff"), m_targetInstance.m_pos + D3DXVECTOR3(-0.3f, 3.5f, -0.5f)));
			SetAnimation("Attack");
			m_attack = true;
			m_attackTime = 0.0f;
		}
	}
	if (KEYDOWN(VK_RETURN))
	{
		if (m_effects.empty())
			m_effects.push_back(new EFFECT_SPELL(m_pDevice, m_magicianTrans.m_pos));
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

	//Set camera
	D3DXMATRIX view, proj, world, identity;
	D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 20.0f, -50.0f);
	D3DXVECTOR3 center = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&view, &eye, &center, &up);
	D3DXMatrixPerspectiveFovLH(&proj, 0.4f, 1.33333f, 0.01f, 1000.0f);
	world = m_magicianTrans.GetWorldMatrix();
	D3DXMatrixIdentity(&identity);

	m_pDevice->SetTransform(D3DTS_VIEW, &view);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);
	m_pDevice->SetTransform(D3DTS_WORLD, &identity);

	// Begin the scene 
	if (SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_unitVS.SetMatrix(m_worldHandle, identity);
		m_unitVS.SetMatrix(m_viewProjHandle, view * proj);

		D3DXVECTOR3 sun;
		D3DXVECTOR3 vec = D3DXVECTOR3(0.5f, 1.0f, -0.5);
		D3DXVec3Normalize(&sun, &vec);
		m_unitVS.SetVector3(m_sunHandle, sun);

		m_unitPS.SetVector4(m_teamColHandle, D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f));

		m_unitVS.Begin();
		m_unitPS.Begin();
		m_skinnedMesh.SetPose(world, m_pAnimControl, m_deltaTime);
		m_skinnedMesh.Render(NULL);
		m_unitPS.End();
		m_unitVS.End();

		m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
		m_targetInstance.Render();

		//Render Effects
		for (int i = 0; i < m_effects.size(); i++)
			if (m_effects[i] != NULL)
				m_effects[i]->Render();

		if (m_effects.empty())
		{
			RECT r[] = { {10, 10, 0, 0}, {10, 30, 0, 0} };
			m_pFont->DrawText(NULL, "Space: Throw Fireball", -1, &r[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
			m_pFont->DrawText(NULL, "Return: Cast Spell", -1, &r[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
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
		UnloadEffectResources();

		m_target.Release();

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

void APPLICATION::SetAnimation(const char name[])
{
	ID3DXAnimationSet* anim = NULL;

	for (int i = 0; i < m_pAnimControl->GetMaxNumAnimationSets(); i++)
	{
		anim = NULL;
		m_pAnimControl->GetAnimationSet(i, &anim);

		if (anim != NULL)
		{
			if (strcmp(name, anim->GetName()) == 0)
				m_pAnimControl->SetTrackAnimationSet(0, anim);
			anim->Release();
		}
	}
}