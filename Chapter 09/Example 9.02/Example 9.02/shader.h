#ifndef CJ_SHADER
#define CJ_SHADER

#include <d3dx9.h>
#include "debug.h"

#define PIXEL_SHADER 0
#define VERTEX_SHADER 1

class SHADER {
public:
	SHADER();
	~SHADER();
	void Init(IDirect3DDevice9* Dev, const char fName[], int typ);

	D3DXHANDLE GetConstant(const char name[]);

	void SetFloat(D3DXHANDLE h, float f);
	void SetVector3(D3DXHANDLE h, D3DXVECTOR3 v);
	void SetVector4(D3DXHANDLE h, D3DXVECTOR4 v);
	void SetMatrix(D3DXHANDLE h, D3DXMATRIX m);

	void Begin();
	void End();

private:

	int type;
	IDirect3DDevice9* Device;
	IDirect3DPixelShader9* PixelShader;
	IDirect3DVertexShader9* VertexShader;
	ID3DXConstantTable* ConstantTable;
};


#endif