#pragma once
struct Rect {
	int left, top, right, bottom;
	Rect() {
		left = right = top = bottom = 0;
	}
	Rect(int l, int t, int r, int b) {
		left = l;
		right = r;
		top = t;
		bottom = b;
	}
	int Width() { return right - left; }
	int Height() { return bottom - top; }
};

struct ViewPort {
	float x;
	float y;
	float width;
	float height;
	float fnear;
	float ffar;
};
struct Plane {
	float a;
	float b;
	float c;
	float d;
};