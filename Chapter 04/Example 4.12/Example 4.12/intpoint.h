#ifndef _INTPOINT
#define _INTPOINT

#include <d3dx9.h>
#include <math.h>
#include "debug.h"

class INTPOINT {
public:
	INTPOINT() { x = y = 0; }
	INTPOINT(int _x, int _y) { Set(_x, _y); }

	void operator=(const POINT rhs) { x = rhs.x; y = rhs.y; }
	bool operator==(const INTPOINT rhs) { return rhs.x == x && rhs.y == y; }
	bool operator!=(const INTPOINT rhs) { return rhs.x != x || rhs.y != y; }
	void operator+=(const INTPOINT rhs) { x += rhs.x; y += rhs.y; }
	void operator/=(const int rhs) { x /= rhs; y /= rhs; }
	INTPOINT operator/(const INTPOINT rhs) { return INTPOINT(x / rhs.x, y / rhs.y); }
	INTPOINT operator/(const int d) { return INTPOINT(x / d, y / d); }
	INTPOINT operator-(const INTPOINT& rhs) { return INTPOINT(x - rhs.x, y - rhs.y); }
	INTPOINT operator+(const INTPOINT& rhs) { return INTPOINT(x + rhs.x, y + rhs.y); }
	INTPOINT operator-(const int& rhs) { return INTPOINT(x - rhs, y - rhs); }
	INTPOINT operator+(const int& rhs) { return INTPOINT(x + rhs, y + rhs); }

	float Distance(INTPOINT p) { return sqrtf((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y)); }
	bool inRect(RECT r) { if (x < r.left || x > r.right || y < r.top || y > r.bottom)return false; else return true; }

	void Set(int _x, int _y) { x = _x; y = _y; }
	int x, y;
};

#endif