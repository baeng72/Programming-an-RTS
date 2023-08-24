#pragma once
#include <common.h>

struct INTPOINT {
	int x, y;
	INTPOINT() {
		x = y = 0;
	}
	INTPOINT(int x_, int y_) {
		x = x_;
		y = y_;
	}
	bool operator==(const INTPOINT& rhs) {
		return (rhs.x == x && rhs.y == y);
	}
	bool operator!=(const INTPOINT& rhs) {
		return !operator==(rhs);
	}
	void operator+=(const INTPOINT& rhs) {
		x += rhs.x;
		y += rhs.y;
	}
	void operator/=(const int rhs) {
		if (rhs != 0) {
			x /= rhs;
			y /= rhs;
		}
	}
	INTPOINT operator/(const INTPOINT& rhs) { return INTPOINT(x / rhs.x, y / rhs.y); }
	INTPOINT operator/(const int d) { return INTPOINT(x / d, y / d); }
	INTPOINT operator-(const INTPOINT& rhs) { return INTPOINT(x - rhs.x, y - rhs.y); }
	INTPOINT operator+(const INTPOINT& rhs) { return INTPOINT(x + rhs.x, y + rhs.y); }
	INTPOINT operator-(const int& rhs) { return INTPOINT(x - rhs, y - rhs); }
	INTPOINT operator+(const int& rhs) { return INTPOINT(x + rhs, y + rhs); }
	float Distance(INTPOINT p) {
		return std::sqrtf((float)((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y)));
	}
	bool inRect(Rect& r) {
		if (x < r.left || x > r.right || y < r.top || y > r.bottom)
			return false;
		return true;
	}
};