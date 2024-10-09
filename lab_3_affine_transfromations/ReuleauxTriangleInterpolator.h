#pragma once
#include "Frame.h"
#include "Matrices.h"

# define PI 3.14159265358979323846 
# define EPS 0.0000000001

// Класс для расчёта треугольника Рёло с закруглением краёв
template <typename ColorInterpolator>
class ReuleauxTriangleInterpolator
{
	Vector B;
	Vector C;
	Vector D;
	Vector E;
	Vector F;
	Vector G;
	Vector H;
	Vector I;
	Vector J;
	Vector O1;
	Vector O2;
	Vector O3;
	double smallCircleRadius;
	double sideSize;
	ColorInterpolator colorInterpolator;
	double x0;
	double y0;
	double x1;
	double y1;
	double x2;
	double y2;
	double wx0, wy0, wx1, wy1, wx2, wy2;
	double S;

	std::pair<bool, Vector> findPointsIntersection(double ax0, double ay0, double ax1, double ay1,
		double bx0, double by0, double bx1, double by1) {
		double adx = ax1 - ax0;
		double ady = ay1 - ay0;
		double bdx = bx1 - bx0;
		double bdy = by1 - by0;
		double denom = bdy * adx - bdx * ady;
		if (denom == 0) {
			return { false, Vector()};
		}
		double t = (bdx * (ay0 - by0) + bdy * (bx0 - ax0)) / denom;
		return { true, Vector(ax0 + adx * t, ay0 + ady * t) };
	}

public:
	ReuleauxTriangleInterpolator(double _x0, double _y0, double _x1, double _y1, double _x2, double _y2, double _wx0, double _wy0, double _wx1, double _wy1,
		double _wx2, double _wy2,
		double rounding, ColorInterpolator colorInterpolator) : colorInterpolator(colorInterpolator),
		wx0(_wx0), wy0(_wy0), wx1(_wx1), wy1(_wy1), wx2(_wx2), wy2(_wy2),
		x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), S((_y1 - _y2)* (_x0 - _x2) + (_x2 - _x1) * (_y0 - _y2)) {
		rounding = max(min(rounding, 1), EPS);
		double roundang = rounding * (PI / 6);
		// Определим основные точки
		double R = .5;
		double a = R * sqrt(3);
		double h = 3 * R / 2;

		// Основные точки для последующей трансформации, см. схему GeoGebra
		this->B = Vector( 0, 0.5 );
		this->C = Vector(sqrt(3) / 4, -0.25 );
		this->D = Vector(-C.x(), C.y());
		this->E = Vector(D.x() + a * cos(roundang), D.y() + a * sin(roundang));
		this->F = Vector(D.x() + a * cos(PI / 3 - roundang), D.y() + a * sin(PI / 3 - roundang));
		this->G = Vector(B.x() + a * cos(-PI / 3 - roundang), B.y() + a * sin(-PI / 3 - roundang));
		this->H = Vector(B.x() + a * cos(-2 * PI / 3 + roundang), B.y() + a * sin(-2 * PI / 3 + roundang));
		this->I = Vector(C.x() + a * cos(2 * PI / 3 + roundang), C.y() + a * sin(2 * PI / 3 + roundang));
		this->J = Vector(C.x() + a * cos(PI - roundang), C.y() + a * sin(PI - roundang));

		this->O1 = findPointsIntersection(D.x(), D.y(), F.x(), F.y(), C.x(), C.y(), I.x(), I.y()).second;
		this->O2 = findPointsIntersection(D.x(), D.y(), E.x(), E.y(), B.x(), B.y(), G.x(), G.y()).second;
		this->O3 = findPointsIntersection(B.x(), B.y(), H.x(), H.y(), C.x(), C.y(), J.x(), J.y()).second;

		this->sideSize = a;
		this->smallCircleRadius = sqrt(pow(F.x() - O1.x(), 2) + pow(F.y() - O1.y(), 2));
	}

	COLOR main(float x, float y) {
		double O1dy = F.y() - I.y();
		double O1dx = F.x() - I.x();
		double O2dy = G.y() - E.y();
		double O2dx = G.x() - E.x();
		double O3dy = J.y() - H.y();
		double O3dx = J.x() - H.x();

		if (pow(x - C.x(), 2) + pow(y - C.y(), 2) <= sideSize * sideSize &&
			pow(x - B.x(), 2) + pow(y - B.y(), 2) <= sideSize * sideSize &&
			pow(x - D.x(), 2) + pow(y - D.y(), 2) <= sideSize * sideSize && 
			(x - I.x()) * O1dy - (y - I.y()) * O1dx >= EPS &&
			(x - E.x()) * O2dy - (y - E.y()) * O2dx >= EPS &&
			(x - H.x()) * O3dy - (y - H.y()) * O3dx >= EPS) {
			return colorInterpolator.color(x, y);
		}
		else if (
			pow(x - O1.x(), 2) + pow(y - O1.y(), 2) <= smallCircleRadius * smallCircleRadius ||
			pow(x - O2.x(), 2) + pow(y - O2.y(), 2) <= smallCircleRadius * smallCircleRadius ||
			pow(x - O3.x(), 2) + pow(y - O3.y(), 2) <= smallCircleRadius * smallCircleRadius) {
				return colorInterpolator.color(x, y);
		}

		return COLOR(0, 0, 0, 0);
	}

	COLOR color(float x, float y) {
		// Барицентрическая интерполяция
		float h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
		float h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
		float h2 = ((y0 - y1) * (x - x1) + (x1 - x0) * (y - y1)) / S;
		//float h2 = 1 - h0 - h1;
		// Если точка (x, y) находится вне треугольника
		if (h0 < -1E-6 || h1 < -1E-6 || h2 < -1E-6)
		{
			return COLOR(0, 0, 0, 0); // Ошибка алгоритма растеризации, если рисуется чёрный пиксель
		}
		// Интерполируем мировые координаты вершин
		float wx = h0 * wx0 + h1 * wx1 + h2 * wx2;
		float wy = h0 * wy0 + h1 * wy1 + h2 * wy2;
		return main(wx, wy);

	}
};
