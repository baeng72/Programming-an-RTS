#ifndef RTS_OBJECT
#define RTS_OBJECT

#include <d3dx9.h>
#include <vector>
#include "functions.h"
#include "debug.h"
#include "mesh.h"

HRESULT LoadObjectResources(IDirect3DDevice9* Device);
void UnloadObjectResources();

#define GNOME 0

class OBJECT {
public:
	OBJECT();
	OBJECT(int t, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 sca, std::string _name);
	void Render();
	void PaintSelected();

	D3DXVECTOR3 GetPosition() { return m_meshInstance.m_pos; }

	MESHINSTANCE m_meshInstance;
	int m_type;
	bool m_selected;
	std::string m_name;
};


#endif