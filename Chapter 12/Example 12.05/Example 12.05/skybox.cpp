#include "skybox.h"

struct SKYBOX_VERTEX
{
	SKYBOX_VERTEX(D3DXVECTOR3 _pos, D3DXVECTOR2 _uv) { pos = _pos; uv = _uv; }
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
	static const DWORD FVF;
};

const DWORD SKYBOX_VERTEX::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

SKYBOX::SKYBOX(IDirect3DDevice9* Dev, const char fileName[], float size)
{
	m_pDevice = Dev;
	std::string endings[] = { "_UP.jpg", "_FR.jpg", "_BK.jpg", "_RT.jpg", "_LF.jpg", "_DN.jpg" };

	//Load the 6 Skybox m_textures
	for (int i = 0; i < 6; i++)
	{
		std::string fName = fileName;
		fName += endings[i];

		IDirect3DTexture9* newTexture = NULL;
		D3DXCreateTextureFromFile(m_pDevice, fName.c_str(), &newTexture);
		m_textures.push_back(newTexture);
	}

	//Create Mesh
	if (FAILED(D3DXCreateMeshFVF(12, 24, D3DXMESH_MANAGED, SKYBOX_VERTEX::FVF, m_pDevice, &m_pMesh)))
		debug.Print("Failed to create SKYBOX m_pMesh");

	//Create vertices
	SKYBOX_VERTEX* v = 0;
	m_pMesh->LockVertexBuffer(0, (void**)&v);

	{
		D3DXVECTOR3 corners[8] = { D3DXVECTOR3(-size,  size,  size),
								D3DXVECTOR3(size,  size,  size),
								D3DXVECTOR3(-size,  size, -size),
								D3DXVECTOR3(size,  size, -size),
								D3DXVECTOR3(-size, -size,  size),
								D3DXVECTOR3(size, -size,  size),
								D3DXVECTOR3(-size, -size, -size),
								D3DXVECTOR3(size, -size, -size) };

		//Up Face
		v[0] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(0.0f, 0.0f));
		v[1] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(1.0f, 0.0f));
		v[2] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(0.0f, 1.0f));
		v[3] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(1.0f, 1.0f));

		//Front Face
		v[4] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(0.0f, 0.0f));
		v[5] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(1.0f, 0.0f));
		v[6] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(0.0f, 1.0f));
		v[7] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(1.0f, 1.0f));

		//Back Face
		v[8] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(0.0f, 0.0f));
		v[9] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(1.0f, 0.0f));
		v[10] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(0.0f, 1.0f));
		v[11] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(1.0f, 1.0f));

		//Right Face
		v[12] = SKYBOX_VERTEX(corners[2], D3DXVECTOR2(0.0f, 0.0f));
		v[13] = SKYBOX_VERTEX(corners[0], D3DXVECTOR2(1.0f, 0.0f));
		v[14] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(0.0f, 1.0f));
		v[15] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(1.0f, 1.0f));

		//Left Face
		v[16] = SKYBOX_VERTEX(corners[1], D3DXVECTOR2(0.0f, 0.0f));
		v[17] = SKYBOX_VERTEX(corners[3], D3DXVECTOR2(1.0f, 0.0f));
		v[18] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(0.0f, 1.0f));
		v[19] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(1.0f, 1.0f));

		//Down Face
		v[20] = SKYBOX_VERTEX(corners[7], D3DXVECTOR2(0.0f, 0.0f));
		v[21] = SKYBOX_VERTEX(corners[6], D3DXVECTOR2(1.0f, 0.0f));
		v[22] = SKYBOX_VERTEX(corners[5], D3DXVECTOR2(0.0f, 1.0f));
		v[23] = SKYBOX_VERTEX(corners[4], D3DXVECTOR2(1.0f, 1.0f));
	}

	m_pMesh->UnlockVertexBuffer();

	//Calculate Indices
	WORD* ind = 0;
	m_pMesh->LockIndexBuffer(0, (void**)&ind);

	int index = 0;
	for (int quad = 0; quad < 6; quad++)
	{
		//First face
		ind[index++] = quad * 4;
		ind[index++] = quad * 4 + 1;
		ind[index++] = quad * 4 + 2;

		//Second Face
		ind[index++] = quad * 4 + 1;
		ind[index++] = quad * 4 + 3;
		ind[index++] = quad * 4 + 2;
	}

	m_pMesh->UnlockIndexBuffer();

	//Set Attributes
	DWORD* att = 0;
	m_pMesh->LockAttributeBuffer(0, &att);

	//Set each quad to its own sub mesh
	for (int i = 0; i < 12; i++)
		att[i] = i / 2;

	m_pMesh->UnlockAttributeBuffer();

	//Set material
	memset(&m_white, 0, sizeof(D3DMATERIAL9));
	m_white.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

SKYBOX::~SKYBOX()
{
	//Release m_textures
	for (int i = 0; i < m_textures.size(); i++)
		if (m_textures[i] != NULL)
			m_textures[i]->Release();
	m_textures.clear();

	//Release mesh
	if (m_pMesh)m_pMesh->Release();
}

void SKYBOX::Render(D3DXVECTOR3 cameraPos)
{
	//Set Renderstates
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, false);

	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	//Set material
	m_pDevice->SetMaterial(&m_white);

	//Move skybox to center on cameraPos
	D3DXMATRIX position;
	D3DXMatrixTranslation(&position, cameraPos.x, cameraPos.y, cameraPos.z);
	m_pDevice->SetTransform(D3DTS_WORLD, &position);

	//Render the six sides of the skybox
	for (int i = 0; i < 6; i++)
	{
		m_pDevice->SetTexture(0, m_textures[i]);
		m_pMesh->DrawSubset(i);
	}

	// Restore render states.
	m_pDevice->SetRenderState(D3DRS_LIGHTING, true);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, true);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void SKYBOX::GenerateEnvironmentMap(D3DXVECTOR3 position, bool saveToFile, TERRAIN& terrain)
{
	//Release m_textures
	for (int i = 0; i < m_textures.size(); i++)
		if (m_textures[i] != NULL)
			m_textures[i]->Release();
	m_textures.clear();

	std::string fNames[] = { "UP.jpg", "FR.jpg", "BK.jpg", "RT.jpg", "LF.jpg", "DN.jpg" };

	//The Projection matrix
	D3DXMATRIX  proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, 1.0f, 1.0f, 1000.0f);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

	D3DXVECTOR3 directions[] = { D3DXVECTOR3(0.0f, 1.0f, 0.0f),		//UP
								D3DXVECTOR3(0.0f, 0.0f, -1.0f),		//Front
								D3DXVECTOR3(0.0f, 0.0f, 1.0f),		//Back
								D3DXVECTOR3(1.0f, 0.0f, 0.0f),		//Right
								D3DXVECTOR3(-1.0f, 0.0f, 0.0f),		//Left
								D3DXVECTOR3(0.0f, -1.0f, 0.0f) };	//Down

	//The camera "UP" direction for the 6 cameras
	D3DXVECTOR3 up[] = { D3DXVECTOR3(0.0f, 0.0f, -1.0f),
						D3DXVECTOR3(0.0f, 1.0f, 0.0f),
						D3DXVECTOR3(0.0f, 1.0f, 0.0f),
						D3DXVECTOR3(0.0f, 1.0f, 0.0f),
						D3DXVECTOR3(0.0f, 1.0f, 0.0f),
						D3DXVECTOR3(0.0f, 0.0f, 1.0f) };

	LPDIRECT3DSURFACE9 backBuffer = NULL;
	m_pDevice->GetRenderTarget(0, &backBuffer);

	for (int i = 0; i < 6; i++)
	{
		//Set the view matrix
		D3DXMATRIX  view;
		D3DXVECTOR3 center = (position + directions[i]);
		D3DXMatrixLookAtLH(&view, &position, &center, &up[i]);
		m_pDevice->SetTransform(D3DTS_VIEW, &view);

		//Create the render target texture & surface
		LPDIRECT3DTEXTURE9 renderTexture = NULL;
		LPDIRECT3DSURFACE9 renderSurface = NULL;
		m_pDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &renderTexture, NULL);
		renderTexture->GetSurfaceLevel(0, &renderSurface);

		//render to the texture
		m_pDevice->SetRenderTarget(0, renderSurface);							//set new render target
		m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0);	//clear texture
		m_pDevice->BeginScene();

		CAMERA cam;
		cam.Init(m_pDevice);
		cam.CalculateFrustum(view, proj);
		terrain.Render(cam, true);

		m_pDevice->EndScene();

		//Save to file
		if (saveToFile)D3DXSaveTextureToFile(fNames[i].c_str(), D3DXIFF_JPG, renderTexture, NULL);

		//Release variables
		renderSurface->Release();
		m_textures.push_back(renderTexture);
	}

	//Restore rendertargets
	m_pDevice->SetRenderTarget(0, backBuffer);
	backBuffer->Release();
}
