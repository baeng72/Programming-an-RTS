#include "functions.h"

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