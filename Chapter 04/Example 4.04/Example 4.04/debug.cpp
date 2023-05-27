#include "debug.h"

std::ofstream out("debug.txt");

DEBUG::DEBUG()
{

}

DEBUG::~DEBUG()
{
	if (out.good())
		out.close();
}

void DEBUG::Print(const char c[])
{
	out << c << std::endl;
}
std::ofstream& DEBUG::operator<<(char c[]) { out << c; return out; }
std::ofstream& DEBUG::operator<<(int i) { out << i; return out; }
std::ofstream& DEBUG::operator<<(float f) { out << f; return out; }
std::ofstream& DEBUG::operator<<(bool b) { if (b)out << "True"; else out << "False"; return out; }
std::ofstream& DEBUG::operator<<(D3DXVECTOR3 v) { out << "x: " << v.x << ", y: " << v.y << ", z: " << v.z; return out; }
void DEBUG::Endl(int nr) { for (int i = 0; i < nr; i++)out << std::endl; }