#ifndef cj_debug
#define cj_debug

#include <d3dx9.h>
#include <fstream>

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

class DEBUG {
public:
	DEBUG();
	~DEBUG();
	void Print(const char c[]);
	std::ofstream& operator<<(char c[]);
	std::ofstream& operator<<(int i);
	std::ofstream& operator<<(float f);
	std::ofstream& operator<<(bool b);
	std::ofstream& operator<<(D3DXVECTOR3 v);
	void Endl(int nr);
};

static DEBUG debug;

#endif