#pragma once

#include "Vector.h"
#include "Color.h"

class BaseShader {
public:
	BaseShader() {}

	virtual COLOR main(Vector data) = 0;
	virtual Vector getVertexData(double h0, double h1, double h2) = 0;
};

class CheckmateShader : public BaseShader {
	Vector A, B, C;

public:
	CheckmateShader(Vector A, Vector B, Vector C) : A(A), B(B), C(C) {}

	COLOR main(Vector data) {
		bool x_factor = (int)(data.x * 5) % 2;
		bool z_factor = (int)(data.z * 5) % 2;
		if ((x_factor + z_factor) % 2) return { 10, 10, 10 };
		return { 255, 255, 255, 120 };
	}

	Vector getVertexData(double h0, double h1, double h2) {
		Vector result = {
			A.x * h0 + B.x * h1 + C.x * h2,
			A.y * h0 + B.y * h1 + C.y * h2,
			A.z * h0 + B.z * h1 + C.z * h2
		};

		return result;
	}
};

class ColorShader : public BaseShader {
public:
	COLOR base;
	ColorShader(COLOR base) : base(base) {}

	COLOR main(Vector data) {
		return base;
	}

	Vector getVertexData(double h0, double h1, double h2) {
		return Vector();
	}
};

class DottedShader : public BaseShader {
	Vector A, B, C;

public:
	DottedShader(Vector A, Vector B, Vector C) : A(A), B(B), C(C) {}

	COLOR main(Vector data) {
		double x_factor = fmod(data.x, 0.2) * 5;
		double z_factor = fmod(data.z, 0.2) * 5;

		if (pow(x_factor - 0.5, 2) + pow(z_factor - 0.5, 2) < 0.25 * 0.25) {
			return { 255, 255, 255 };
		}
		else {
			return { 255, 0, 0, 255};
		}
	}

	Vector getVertexData(double h0, double h1, double h2) {
		Vector result = {
			A.x * h0 + B.x * h1 + C.x * h2,
			A.y * h0 + B.y * h1 + C.y * h2,
			A.z * h0 + B.z * h1 + C.z * h2
		};

		return result;
	}
};