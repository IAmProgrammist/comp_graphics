#pragma once

#include <vector>
#include <tuple>
#include "Vector.h"


// Возвращает точки объекта
const std::vector<Vector> get_points(float scale_factor = 1.) {
	std::vector<Vector> result = {
		// Вершина рукоятки
		{-0.05, 1, 0.05}, // 0
		{0.05, 1, 0.05},  // 1
		{-0.05, 1, -0.05},// 2
		{0.05, 1, -0.05}, // 3

		// Нижняя часть рукоятки
		{-0.05, 0.25, 0.05}, // 4
		{0.05, 0.25, 0.05},  // 5
		{-0.05, 0.25, -0.05},// 6
		{0.05, 0.25, -0.05}, // 7

		// Нижняя часть молота
		{-0.4, 0.25, 0.2}, // 8
		{0.4, 0.25, 0.2},  // 9
		{-0.4, 0.25, -0.2},// 10
		{0.4, 0.25, -0.2}, // 11

		// Боковые шипы молота
		{-(0.4f + scale_factor), 0.17, 0}, // 12
		{(0.4f + scale_factor), 0.17, 0},  // 13
		{0, 0.17, -(0.2f + scale_factor)},                // 14
		{0, 0.17, (0.2f + scale_factor)},                 // 15

		// Соединители боковых шипов
		{-0.4, 0.09, 0.2},  // 16
		{0.4, 0.09, 0.2},   // 17
		{-0.4, 0.09, -0.2}, // 18
		{0.4, 0.09, -0.2},  // 19

		// Нижний шип
		{0, 0.09f - scale_factor, 0}   // 20
	};

	return result;
}

// Задаёт индексы точек для формирования полигонов
std::vector<std::tuple<int, int, int>> polygons = {
	// Вершина рукоятки
	{0, 1, 2},
	{3, 1, 2},

	// Стенки рукоятки
	{0, 1, 5},
	{1, 5, 7},
	{1, 3, 7},
	{3, 7, 6},
	{3, 2, 6},
	{2, 6, 4},
	{2, 0, 4},
	{0, 4, 5},

	// Нижняя часть рукоятки
	{5, 4, 9},
	{4, 9, 8},
	{6, 4, 8},
	{6, 8, 10},
	{6, 7, 10},
	{7, 10, 11},
	{7, 11, 5},
	{11, 5, 9},

	// Шипы молота
	{8, 10, 12},
	{8, 9, 15},
	{9, 11, 13},
	{11, 10, 14},
	{16, 18, 12},
	{16, 17, 15},
	{17, 19, 13},
	{19, 18, 14},

	{11, 19, 13},
	{11, 19, 14},

	{10, 18, 14},
	{10, 18, 12},

	{8, 16, 12},
	{8, 16, 15},

	{9, 17, 15},
	{9, 17, 13},

	// Нижний шип
	{19, 18, 20},
	{18, 16, 20},
	{16, 17, 20},
	{17, 19, 20}
};

std::vector<COLOR> materials = {
	{132, 39, 75},
	{126, 142, 215},
	{98, 82, 154},
	{142, 132, 211},
	{97, 103, 222},
	{70, 93, 36},
	{62, 122, 190},
	{83, 57, 98},
	{148, 167, 154},
	{111, 115, 224},
	{175, 218, 20},
	{168, 178, 84},
	{40, 168, 153},
	{132, 46, 214},
	{105, 215, 86},
	{109, 183, 95},
	{64, 121, 201},
	{141, 185, 103},
	{88, 116, 207},
	{42, 205, 195},
	{143, 190, 44},
	{218, 156, 52},
	{205, 68, 192},
	{61, 68, 142},
	{96, 129, 219},
	{196, 61, 81},
	{139, 69, 110},
	{205, 210, 134},
	{184, 158, 46},
	{225, 131, 143},
	{168, 156, 159},
	{143, 67, 152},
	{160, 210, 114},
	{125, 75, 154},
	{32, 191, 21},
	{73, 216, 93},
	{112, 113, 173},
	{146, 211, 170}
};

/*const std::vector<Vector> get_points(float scale_factor = 1.) {
	std::vector<Vector> result = {
		{0, 0, 0},
		{1, 0, 0},
		{1, 0, 1},
		{0, 0, 1},
		{0, 1, 0},
		{1, 1, 0},
		{1, 1, 1},
		{0, 1, 1},
	};

	return result;
};

std::vector<std::tuple<int, int, int>> polygons = {
	{0, 1, 2}, 
	{0, 3, 2},
	{4, 5, 6},
	{4, 7, 6},
	{0, 4, 3},
	{4, 7, 3},
	{0, 4, 1},
	{5, 4, 1},
	{5, 2, 1},
	{5, 2, 6},
	{2, 6, 3},
	{7, 6, 3},
};

std::vector<COLOR> materials = {
	{255, 0, 0},
	{255, 0, 0},
	{0, 255, 0},
	{0, 255, 0},
	{0, 0, 255},
	{0, 0, 255},
	{255, 255, 0},
	{255, 255, 0},
	{0, 255, 255},
	{0, 255, 255},
	{255, 0, 255},
	{255, 0, 255}
};*/