#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"

float global_angle = 0;

class Painter
{
public:

	void Draw(Frame& frame)
	{
	
		int W = frame.width, H = frame.height;

		// Размер рисунка возьмём меньше (7 / 8), чтобы он не касался границ экрана 
		float a = 7.0 / 8.0 * ((W < H) ? W - 1 : H - 1) / sqrt(2);

		float angle = global_angle; // Угол поворота
		a = a / 2;
		
		// Инициализируем исходные координаты центра и вершин квадрата
		struct
		{
			float x;
			float y;
		} C = {W / 2 + 0.5f, H / 2 + 0.5f}, A[4] = { { C.x + a, C.y + a}, {C.x + a, C.y - a}, {C.x - a, C.y - a}, {C.x - a, C.y + a} };
	

		// Поворачиваем все вершины квадрата вокруг точки C на угол angle
		for (int i = 0; i < 4; i++)
		{
			double xi = A[i].x, yi = A[i].y;
			A[i].x = (xi - C.x) * cos(angle) - (yi - C.y) * sin(angle) + C.x;
			A[i].y = (xi - C.x) * sin(angle) + (yi - C.y) * cos(angle) + C.y;
		}

	
		// Класс для расчёта барицентрической интерполяции
		class BarycentricInterpolator
		{
			float x0, y0, x1, y1, x2, y2, S;
			COLOR C0, C1, C2;

			public:
			BarycentricInterpolator(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2, COLOR A0, COLOR A1, COLOR A2) :
				x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), 
				S((_y1 - _y2)*(_x0 - _x2) + (_x2 - _x1)*(_y0 - _y2)), C0(A0), C1(A1), C2(A2)
			{
			}


			COLOR color(float x, float y)
			{
				// Барицентрическая интерполяция
				float h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
				float h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
				float h2 = 1 - h0 - h1;
				float r = h0 * C0.RED   + h1 * C1.RED   + h2 * C2.RED;
				float g = h0 * C0.GREEN + h1 * C1.GREEN + h2 * C2.GREEN;
				float b = h0 * C0.BLUE  + h1 * C1.BLUE  + h2 * C2.BLUE;
				float a = h0 * C0.ALPHA + h1 * C1.ALPHA + h2 * C2.ALPHA;
				// Из-за погрешности аппроксимации треугольника учитываем, что центр закрашиваемого пикселя может находится вне треугольника.
				// По этой причине значения r, g, b могут выйти за пределы диапазона [0, 255].
				return COLOR(r, g, b, a);
			}
		};

	
		// Класс для расчёта радиальной интерполяции
		class RadialInterpolator
		{
			float cx, cy; // Центр прямоугольника
			COLOR C0, C1; // Цвета радиальной заливки
			float angle;  // Начальный угол заливки

		public:
			RadialInterpolator(float _x0, float _y0, float _x1, float _y1, COLOR A0, COLOR A1, float _angle) :
				cx((_x0 + _x1) / 2.0f), cy((_y0 + _y1)/ 2.0f),
				C0(A0), C1(A1), angle(_angle)
			{
			}


			COLOR color(float x, float y)
			{
				double dx = (double)x - cx, dy = (double)y - cy;
				double radius = sqrt(dx*dx + dy*dy);

				float h0 = (sin(radius / 10 + angle) + 1.0f) / 2;
				float h1 = 1 - h0;

				float r = h0 * C0.RED + h1 * C1.RED;
				float g = h0 * C0.GREEN + h1 * C1.GREEN;
				float b = h0 * C0.BLUE + h1 * C1.BLUE;

				return COLOR(r, g, b);

			}
		};

		// Рисование прямоугольника с радиальной заливкой
		float x0 = 0, y0 = 0, x1 = frame.width, y1 = frame.height;
		RadialInterpolator radialInterpolator(x0, y0, x1, y1, COLOR(255, 0, 0), COLOR(100, 20, 0), global_angle);
		frame.Triangle(x0, y0, x0, y1, x1, y0, radialInterpolator);
		frame.Triangle(x0, y1, x1, y0, x1, y1, radialInterpolator);

		// Рисование полупрозрачного квадрата
		BarycentricInterpolator interpolator1(A[3].x, A[3].y, A[2].x, A[2].y, A[0].x, A[0].y, COLOR(0, 255, 0, 200), COLOR(255, 0, 0, 200), COLOR(0, 0, 255, 200));
		frame.Triangle(A[3].x, A[3].y, A[2].x, A[2].y, A[0].x, A[0].y, interpolator1);

		BarycentricInterpolator interpolator2(A[0].x, A[0].y, A[2].x, A[2].y, A[1].x, A[1].y, COLOR(0, 0, 255, 200), COLOR(255, 0, 0, 200), COLOR(0, 255, 0, 200));
		frame.Triangle(A[0].x, A[0].y, A[2].x, A[2].y, A[1].x, A[1].y, interpolator2);
		
	}
};

#endif // PAINTER_H