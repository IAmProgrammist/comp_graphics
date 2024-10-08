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
	ReuleauxTriangleInterpolator(double x, double y, double size, double angle, double rounding, ColorInterpolator colorInterpolator) : colorInterpolator(colorInterpolator) {
		angle = angle < 0 ? (2 * PI / 3. + fmod(angle, 2 * PI / 3.)) : (fmod(angle, 2 * PI / 3.));
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

		Matrix S = Matrix::scale(size);
		Matrix Ro = Matrix::rotation(angle);
		Matrix T = Matrix::transfrom(x, y);
		Matrix SRT = (T * Ro) * S;

		B = SRT * B;
		C = SRT * C;
		D = SRT * D;
		E = SRT * E;
		F = SRT * F;
		G = SRT * G;
		H = SRT * H;
		I = SRT * I;
		J = SRT * J;
		O1 = SRT * O1;
		O2 = SRT * O2;
		O3 = SRT * O3;

		this->sideSize = a * size;
		this->smallCircleRadius = sqrt(pow(F.x() - O1.x(), 2) + pow(F.y() - O1.y(), 2));
	}

	COLOR color(float x, float y) {
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
};
