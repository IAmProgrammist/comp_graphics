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
		return HSVCOLOR(180 + (getangle(x, y) * 180) / 3.14, 1, 1).convertToRgb();
	}
};
