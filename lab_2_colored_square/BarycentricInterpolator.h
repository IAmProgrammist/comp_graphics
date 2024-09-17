#pragma once
#include "Frame.h"

// Класс для расчёта барицентрической интерполяции
class BarycentricInterpolator
{
	float x0, y0, x1, y1, x2, y2, S;
	COLOR C0, C1, C2;

public:
	BarycentricInterpolator(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2, COLOR A0, COLOR A1, COLOR A2) :
		x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2),
		S((_y1 - _y2)* (_x0 - _x2) + (_x2 - _x1) * (_y0 - _y2)), C0(A0), C1(A1), C2(A2)
	{
	}


	COLOR color(float x, float y)
	{
		// Барицентрическая интерполяция
		float h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
		float h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
		float h2 = 1 - h0 - h1;
		float r = h0 * C0.RED + h1 * C1.RED + h2 * C2.RED;
		float g = h0 * C0.GREEN + h1 * C1.GREEN + h2 * C2.GREEN;
		float b = h0 * C0.BLUE + h1 * C1.BLUE + h2 * C2.BLUE;
		float a = h0 * C0.ALPHA + h1 * C1.ALPHA + h2 * C2.ALPHA;
		// Из-за погрешности аппроксимации треугольника учитываем, что центр закрашиваемого пикселя может находится вне треугольника.
		// По этой причине значения r, g, b могут выйти за пределы диапазона [0, 255].
		return COLOR(r, g, b, a);
	}
};
