#pragma once
#include "Frame.h"

// Класс для расчёта барицентрической интерполяции
class SectorInterpolator
{
	float c_x, c_y;
	COLOR C0, C1, C2;


	inline double getangle(int x, int y) {
		double x1 = 0;
		double y1 = 1;
		double x2 = x - c_x;
		double y2 = y - c_y;

		double dot = x1 * x2 + y1 * y2;
		double det = x1 * y2 - y1 * x2;
		return atan2(det, dot);
	}
public:
	SectorInterpolator(float c_x, float c_y) :
		c_x(c_x), c_y(c_y)
	{
	}


	COLOR color(float x, float y)
	{
		int r = 255, g = 0, b = 0;
		double angle = 180 + (getangle(x, y) * 180) / 3.14;
		if (angle >= 0 && angle < 60) {
			r = 255;
			g = (angle / 60.0) * 255;
			b = 0;
		}
		else if (angle >= 60 && angle < 120) {
			angle -= 60;
			r = (1 - angle / 60.0) * 255;
			g = 255;
			b = 0;
		}
		else if (angle >= 120 && angle < 180) {
			angle -= 120;
			r = 0;
			g = 255;
			b = (angle / 60.0) * 255;
		}
		else if (angle >= 180 && angle < 240) {
			angle -= 180;
			r = 0;
			g = (1 - angle / 60.0) * 255;
			b = 255;
		}
		else if (angle >= 240 && angle < 300) {
			angle -= 240;
			r = (angle / 60.0) * 255;
			g = 0;
			b = 255;
		}
		else if (angle >= 300 && angle < 360) {
			angle -= 300;
			r = 255;
			g = 0;
			b = (1 - angle / 60.0) * 255;
		}

		return COLOR(r, g, b);
	}
};
