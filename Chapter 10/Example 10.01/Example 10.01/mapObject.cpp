#include "mapObject.h"
#include "unit.h"

//Line interface to draw selected units/buildings with
ID3DXLine* line = NULL;

//variables used for fog-of-war
IDirect3DTexture9* sightTexture = NULL;
ID3DXMesh* sightMesh = NULL;
D3DMATERIAL9 sightMtrl;

struct SIGHTVertex
{
	SIGHTVertex() {}
	SIGHTVertex(D3DXVECTOR3 pos, D3DXVECTOR2 _uv)
	{
		position = pos;
		uv = _uv;
	}

	D3DXVECTOR3 position;
	D3DXVECTOR2 uv;

	static const DWORD FVF;
};

const DWORD SIGHTVertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

void LoadMapObjectResources(IDirect3DDevice9* Device)
{
	D3DXCreateLine(Device, &line);

	//Sight texture
	Device->CreateTexture(64, 64, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &sightTexture, NULL);

	D3DLOCKED_RECT sRect;
	sightTexture->LockRect(0, &sRect, NULL, NULL);
	BYTE* bytes = (BYTE*)sRect.pBits;
	//memset(bytes, 0, sRect.Pitch * sRect.Pitch);
	memset(bytes, 0, sRect.Pitch * 64);

	float intensity = 1.3f;
	D3DXVECTOR2 center = D3DXVECTOR2(32.0f, 32.0f);

	for (int y = 0; y < 64; y++)
		for (int x = 0; x < 64; x++)
		{
			D3DXVECTOR2 vec = (center - D3DXVECTOR2(x, y));
			float d = D3DXVec2Length(&vec);
			int value = ((32.0f - d) / 32.0f) * 255.0f * intensity;
			if (value < 0)value = 0;
			if (value > 255)value = 255;
			bytes[x + y * sRect.Pitch] = value;
		}
	sightTexture->UnlockRect(0);

	//D3DXSaveTextureToFile("sightTexture.bmp", D3DXIFF_BMP, sightTexture, NULL);

	//Calculate sight mesh (a simple quad)
	D3DXCreateMeshFVF(2, 4, D3DXMESH_MANAGED, SIGHTVertex::FVF, Device, &sightMesh);

	//Create 4 vertices
	SIGHTVertex* v = 0;
	sightMesh->LockVertexBuffer(0, (void**)&v);
	v[0] = SIGHTVertex(D3DXVECTOR3(-1, 0, 1), D3DXVECTOR2(0, 0));
	v[1] = SIGHTVertex(D3DXVECTOR3(1, 0, 1), D3DXVECTOR2(1, 0));
	v[2] = SIGHTVertex(D3DXVECTOR3(-1, 0, -1), D3DXVECTOR2(0, 1));
	v[3] = SIGHTVertex(D3DXVECTOR3(1, 0, -1), D3DXVECTOR2(1, 1));
	sightMesh->UnlockVertexBuffer();

	//Create 2 faces
	WORD* indices = 0;
	sightMesh->LockIndexBuffer(0, (void**)&indices);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 1; indices[4] = 3; indices[5] = 2;
	sightMesh->UnlockIndexBuffer();

	//Set Attributes for the 2 faces
	DWORD* att = 0;
	sightMesh->LockAttributeBuffer(0, &att);
	att[0] = 0; att[1] = 0;
	sightMesh->UnlockAttributeBuffer();

	//Sight MTRL
	memset(&sightMtrl, 0, sizeof(D3DMATERIAL9));
	sightMtrl.Diffuse = sightMtrl.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

void UnloadMapObjectResources()
{
	if (line)line->Release();
	line = NULL;

	if (sightTexture)sightTexture->Release();
	sightTexture = NULL;

	if (sightMesh)sightMesh->Release();
	sightMesh = NULL;
}

INTPOINT GetScreenPos(D3DXVECTOR3 pos, IDirect3DDevice9* Device)
{
	D3DXVECTOR3 screenPos;
	D3DVIEWPORT9 Viewport;
	D3DXMATRIX Projection, View, World;

	Device->GetViewport(&Viewport);
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	D3DXMatrixIdentity(&World);
	D3DXVec3Project(&screenPos, &pos, &Viewport, &Projection, &View, &World);

	return INTPOINT(screenPos.x, screenPos.y);
}

//////////////////////////////////////////////////////////////////////////////////
//								MapObject										//
//////////////////////////////////////////////////////////////////////////////////

MAPOBJECT::MAPOBJECT()
{
	//Sets all variables to 0, NULL or False
	m_isBuilding = false;
	m_pTerrain = NULL;
	m_hp = m_hpMax = 0;
	m_range = m_damage = 0;
	m_sightRadius = 0.0f;
	m_team = m_type = 0;
	m_selected = m_dead = m_visible = false;
	m_pTarget = NULL;
	m_pDevice = NULL;
}

RECT MAPOBJECT::GetMapRect(int border)
{
	RECT mr = { m_mappos.x - border,
			   m_mappos.y - border,
			   m_mappos.x + m_mapsize.x + border,
			   m_mappos.y + m_mapsize.y + border };

	return mr;
}

void MAPOBJECT::PaintSelected()
{
	if (!m_selected || m_pDevice == NULL)return;

	BBOX bbox = GetBoundingBox();	//Bounding box in world space

	// Create 8 points according to the corners of the bounding box
	D3DXVECTOR3 corners[] = { D3DXVECTOR3(bbox.max.x, bbox.max.y, bbox.max.z),
							 D3DXVECTOR3(bbox.max.x, bbox.max.y, bbox.min.z),
							 D3DXVECTOR3(bbox.max.x, bbox.min.y, bbox.max.z),
							 D3DXVECTOR3(bbox.max.x, bbox.min.y, bbox.min.z),
							 D3DXVECTOR3(bbox.min.x, bbox.max.y, bbox.max.z),
							 D3DXVECTOR3(bbox.min.x, bbox.max.y, bbox.min.z),
							 D3DXVECTOR3(bbox.min.x, bbox.min.y, bbox.max.z),
							 D3DXVECTOR3(bbox.min.x, bbox.min.y, bbox.min.z) };

	// Find the max and min points of these
	// 8 offests points in screen space
	INTPOINT pmax(-10000, -10000), pmin(10000, 10000);

	for (int i = 0; i < 8; i++)
	{
		INTPOINT screenPos = GetScreenPos(corners[i], m_pDevice);

		if (screenPos.x > pmax.x)pmax.x = screenPos.x;
		if (screenPos.y > pmax.y)pmax.y = screenPos.y;
		if (screenPos.x < pmin.x)pmin.x = screenPos.x;
		if (screenPos.y < pmin.y)pmin.y = screenPos.y;
	}

	RECT scr = { -20, -20, 820, 620 };

	// Check that the max and min point is within our viewport boundaries
	if (pmax.inRect(scr) || pmin.inRect(scr))
	{
		float s = (pmax.x - pmin.x) / 3.0f;
		if ((pmax.y - pmin.y) < (pmax.x - pmin.x))s = (pmax.y - pmin.y) / 3.0f;

		D3DXVECTOR2 corner1[] = { D3DXVECTOR2(pmin.x, pmin.y + s), D3DXVECTOR2(pmin.x, pmin.y), D3DXVECTOR2(pmin.x + s, pmin.y) };
		D3DXVECTOR2 corner2[] = { D3DXVECTOR2(pmax.x - s, pmin.y), D3DXVECTOR2(pmax.x, pmin.y), D3DXVECTOR2(pmax.x, pmin.y + s) };
		D3DXVECTOR2 corner3[] = { D3DXVECTOR2(pmax.x, pmax.y - s), D3DXVECTOR2(pmax.x, pmax.y), D3DXVECTOR2(pmax.x - s, pmax.y) };
		D3DXVECTOR2 corner4[] = { D3DXVECTOR2(pmin.x + s, pmax.y), D3DXVECTOR2(pmin.x, pmax.y), D3DXVECTOR2(pmin.x, pmax.y - s) };

		//Draw the 4 corners
		if (line != NULL)
		{
			line->SetWidth(2.0f);
			line->Begin();
			line->Draw(corner1, 3, 0xffffffff);
			line->Draw(corner2, 3, 0xffffffff);
			line->Draw(corner3, 3, 0xffffffff);
			line->Draw(corner4, 3, 0xffffffff);
			line->End();
		}
	}
}

void MAPOBJECT::RenderSightMesh()
{
	if (m_pDevice == NULL || sightTexture == NULL || sightMesh == NULL)return;

	//Set world transformation matrix
	D3DXMATRIX world, pos, sca;

	//Position the mesh at the center of the map object
	D3DXMatrixTranslation(&pos, m_position.x, m_position.y, m_position.z);

	//Scale the mesh to the sight radius of the mapobject (XZ plane)
	D3DXMatrixScaling(&sca, m_sightRadius, 1.0f, m_sightRadius);

	D3DXMatrixMultiply(&world, &sca, &pos);
	m_pDevice->SetTransform(D3DTS_WORLD, &world);

	//Set texture and material
	m_pDevice->SetTexture(0, sightTexture);
	m_pDevice->SetMaterial(&sightMtrl);

	//Draw the sight mesh
	sightMesh->DrawSubset(0);
}