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
};