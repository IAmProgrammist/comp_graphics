#pragma once
#include "Frame.h"

// Класс для расчёта радиальной интерполяции
class RadialInterpolator
{
	float cx, cy; // Центр прямоугольника
	std::vector<COLOR> colors; // Цвета радиальной заливки
	float angle;  // Начальный угол заливки

public:
	RadialInterpolator(float _x0, float _y0, float _x1, float _y1, COLOR A0, COLOR A1, float _angle) :
		cx((_x0 + _x1) / 2.0f), cy((_y0 + _y1) / 2.0f),
		colors({A0, A1}), angle(_angle)
	{
	}
	RadialInterpolator(float _x0, float _y0, float _x1, float _y1, std::vector<COLOR> _colors, float _angle) :
		cx((_x0 + _x1) / 2.0f), cy((_y0 + _y1) / 2.0f),
		colors(_colors), angle(_angle)
	{
	}

	COLOR color(float x, float y)
	{
		double dx = (double)x - cx, dy = (double)y - cy;
		double radius = sqrt(dx * dx + dy * dy);

		double h0 = radius / 40 + angle;
		h0 -= floor(h0);

		h0 *= colors.size();
		int h0IndexColors = h0;
		int h1IndexColors = h0IndexColors + 1;
		COLOR colorh01 = colors[h0IndexColors % colors.size()];
		COLOR colorh11 = colors[h1IndexColors % colors.size()];
		h0 -= floor(h0);
		double h1 = 1 - h0;

		double r = h0 * colorh11.RED +h1 * colorh01.RED;
		double g = h0 * colorh11.GREEN +h1 * colorh01.GREEN;
		double b = h0 * colorh11.BLUE +h1 * colorh01.BLUE;

		return COLOR(r, g, b);

	}
};
