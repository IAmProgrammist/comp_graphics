#pragma once
#pragma once

#include <string>
#include <vector>

class Vector {
public:
	double vector[3];
	Vector(std::initializer_list<double> v) {
		memcpy(vector, v.begin(), sizeof(double) * 3);
	}
	Vector(std::vector<double> v) {
		memcpy(vector, &v[0], sizeof(double) * 3);
	}
};

class Matrix {
public:
	double data[9];
	double* matrix[3];
	Matrix(std::initializer_list<double> v) {
		memcpy(data, v.begin(), sizeof(double) * 9);
		matrix[0] = data;
		matrix[1] = data + 3;
		matrix[2] = data + 6;
	}
	Matrix(std::vector<double> v) {
		memcpy(data, &v[0], sizeof(double) * 9);
		matrix[0] = data;
		matrix[1] = data + 3;
		matrix[2] = data + 6;
	}

	Matrix multiply(Matrix& another) {
		double dataNew[9] = {};
		double* matrixNew[3];
		matrixNew[0] = dataNew;
		matrixNew[1] = dataNew + 3;
		matrixNew[2] = dataNew + 6;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				matrixNew[i][j] = 0;

				for (int k = 0; k < 3; k++) {
					matrixNew[i][j] += this->matrix[i][k] * another.matrix[k][j];
				}
			}
		}

		return Matrix(std::vector<double>(dataNew, dataNew + 9));
	}

	Vector multiply(Vector& vec) {
		return Vector({
			vec.vector[0] * this->matrix[0][0] + vec.vector[1] * this->matrix[0][1] + vec.vector[2] * this->matrix[0][2],
			vec.vector[0] * this->matrix[1][0] + vec.vector[1] * this->matrix[1][1] + vec.vector[2] * this->matrix[1][2],
			vec.vector[0] * this->matrix[2][0] + vec.vector[1] * this->matrix[2][1] + vec.vector[2] * this->matrix[2][2] });
	}
};