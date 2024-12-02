#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"
#include "Vector.h"
#include "Model.h"
#include <optional>
#include <tuple>

// Время от начала запуска программы
float time = 0;
float scale = .1;
float x_offset = 0;
float y_offset = 0;
float z_offset = -2;
float x_rot = 0;
float y_rot = 0;
float z_rot = 0;
float fig_scale = 2.5;
int draw_mode = 1;
Perspective currentPerspective = static_cast<Perspective>(0);

#define EPS 0.0000000001

std::tuple<bool, Vector> intersect_points_2d(Vector A, Vector B, Vector C, Vector D) {
	float dABx = std::abs(B.x - A.x);
	float dCDx = std::abs(C.x - D.x);
	float dABy = std::abs(B.y - A.y);
	float dCDy = std::abs(C.y - D.y);
	Vector O = {};

	if (dABx < EPS && dCDy < EPS) {
		O.x = A.x;
		O.y = C.y;

		return { true, O };
	}
	else if (dABy < EPS && dCDx < EPS) {
		O.x = C.x;
		O.y = A.y;

		return { true, O };
	}
	if (dABx < EPS && dCDx < EPS || dABy < EPS && dCDy < EPS) {
		// Прямые параллельны
		return { false, O };
	}
	else if (dABx < EPS && dABy < EPS || dCDx < EPS && dCDy < EPS) {
		// Передана точка
		return { false, O };
	}
	else if (dABx < EPS) {
		// AB параллельна абсциссе
		O.x = A.x;
		O.y = (O.x - C.x) * (D.y - C.y) / (D.x - C.x) + C.y;

		return {
			O.x < max(C.x, D.x) && O.x > min(C.x, D.x) &&
			O.y < max(A.y, B.y) && O.y > min(A.y, B.y) &&
			O.y < max(C.y, D.y) && O.y > min(C.y, D.y), O };
	}
	else if (dABy < EPS) {
		// AB параллельна ординате
		O.y = A.y;
		O.x = (O.y - C.y) * (D.x - C.x) / (D.y - C.y) + C.x;

		return { O.x < max(A.x, B.x) && O.x > min(A.x, B.x) &&
			O.x < max(C.x, D.x) && O.x > min(C.x, D.x) &&
			O.y < max(C.y, D.y) && O.y > min(C.y, D.y), O };
	}
	else if (dCDy < EPS) {
		O.y = C.y;
		O.x = (O.y - A.y) * (B.x - A.x) / (B.y - A.y) + A.x;

		return { O.x < max(A.x, B.x) && O.x > min(A.x, B.x) &&
			O.x < max(C.x, D.x) && O.x > min(C.x, D.x) &&
			O.y < max(A.y, B.y) && O.y > min(A.y, B.y), O };
	}
	else if (dCDx < EPS) {
		O.x = C.x;
		O.y = (O.x - A.x) * (B.y - A.y) / (B.x - A.x) + A.y;

		return { O.x < max(A.x, B.x) && O.x > min(A.x, B.x) &&
			O.y < max(A.y, B.y) && O.y > min(A.y, B.y) &&
			O.y < max(C.y, D.y) && O.y > min(C.y, D.y), O };
	}
	else {
		O.y = ((D.y - C.y) * (A.x - A.y * (B.x - A.x) / (B.y - A.y) - C.x) / (D.x - C.x) + C.y) / (1 - (D.y - C.y) * (B.x - A.x) / ((D.x - C.x) * (B.y - A.y)));
		O.x = (O.y - C.y) * (D.x - C.x) / (D.y - C.y) + C.x;

		return { O.x < max(A.x, B.x) && O.x > min(A.x, B.x) &&
			O.x < max(C.x, D.x) && O.x > min(C.x, D.x) &&
			O.y < max(A.y, B.y) && O.y > min(A.y, B.y) &&
			O.y < max(C.y, D.y) && O.y > min(C.y, D.y), O };
	}
}

// Возвращает 1, если AC ближе CD.
// Возвращает -1, еслир AC дальше CD.
// Возвращает 0, если отрезки не пересекаются.
int cmp_sides(Vector A, Vector B, Vector C, Vector D) {
	auto inters_res = intersect_points_2d({
		A.x, A.y, 0, 0
		}, {
			B.x, B.y, 0, 0
		}, {
			C.x, C.y, 0, 0
		}, {
			D.x, D.y, 0, 0
		});

	// Если точки совпадают, нужно вернуть 0, иначе всё ломается (очень плохо ломается)
	if (std::abs(A.x - C.x) < EPS && std::abs(A.y - C.y) < EPS ||
		std::abs(A.x - D.x) < EPS && std::abs(A.y - D.y) < EPS ||
		std::abs(B.x - C.x) < EPS && std::abs(B.y - C.y) < EPS ||
		std::abs(B.x - D.x) < EPS && std::abs(B.y - D.y) < EPS) return 0;

	if (!std::get<0>(inters_res)) return 0;

	auto O = std::get<1>(inters_res);
	float zAB, zCD;

	if (std::abs(B.x - A.x) > std::abs(B.y - A.y)) {
		zAB = (O.x - A.x) * (B.z - A.z) / (B.x - A.x) + A.z;
	}
	else {
		zAB = (O.y - A.y) * (B.z - A.z) / (B.y - A.y) + A.z;
	}

	if (std::abs(D.x - C.x) > std::abs(D.y - C.y)) {
		zCD = (O.x - C.x) * (D.z - C.z) / (D.x - C.x) + C.z;
	}
	else {
		zCD = (O.y - C.y) * (D.z - C.z) / (D.y - C.y) + C.z;
	}

	if (zAB < zCD) {
		return 1;
	}
	else {
		return -1;
	}
}

bool is_point_inside_polygon(Vector O, std::vector<Vector> polygon) {
	if (std::abs(O.x - polygon[0].x) < EPS && std::abs(O.x - polygon[0].x) < EPS ||
		std::abs(O.x - polygon[1].x) < EPS && std::abs(O.x - polygon[1].x) < EPS ||
		std::abs(O.x - polygon[2].x) < EPS && std::abs(O.x - polygon[2].x) < EPS) return true;

	int neg_count = 0, pos_count = 0;
	std::vector<Vector> vectors;
	for (int i = 0; i < polygon.size(); i++) {
		vectors.push_back({
			polygon[i].x - O.x,
			polygon[i].y - O.y,
			polygon[i].z - O.z
		});
	}

	for (int i = 0; i < polygon.size(); i++) {
		Vector a = vectors[i], b = vectors[(i + 1) % polygon.size()];
		float cz = a.x * b.y - b.x * a.y;
		if (cz < 0) {
			neg_count++;
		}
		else {
			pos_count++;
		}
	}

	return neg_count == polygon.size() || pos_count == polygon.size();
}

// Возвращает 1, если A ближе B
// Возвращает -1, если B ближе A
// Возвращает 0, если A и B не пересекаются.
int cmp_triangles(std::vector<Vector> polygonA, std::vector<Vector> polygonB) {
	Vector Ea;
	Vector Eb;
	bool Eb_first = true;
	bool a_in_b = true;
	bool b_in_a = true;

	for (int i = 0; i < polygonA.size(); i++) {
		Ea.x += polygonA[i].x / polygonA.size();
		Ea.y += polygonA[i].y / polygonA.size();
		Ea.z += polygonA[i].z / polygonA.size();
		a_in_b &= is_point_inside_polygon(polygonA[i], polygonB);

		for (int j = 0; j < polygonB.size(); j++) {
			auto compare_sides = cmp_sides(
				polygonA[i], polygonA[(i + 1) % polygonA.size()],
				polygonB[j], polygonB[(j + 1) % polygonB.size()]
			);

			if (compare_sides) return compare_sides;

			if (!Eb_first) continue;

			Eb.x += polygonB[j].x / polygonB.size();
			Eb.y += polygonB[j].y / polygonB.size();
			Eb.z += polygonB[j].z / polygonB.size();

			b_in_a &= is_point_inside_polygon(polygonB[j], polygonA);
		}

		Eb_first = false;
	}

	bool swapped = false;

	if (!a_in_b && !b_in_a) return 0;

	// Делаем так, чтобы b был внутри a
	if (a_in_b) {
		std::swap(Ea, Eb);
		std::swap(polygonA, polygonB);
		swapped = true;
	}

	auto P0 = polygonA[0];
	auto P1 = polygonA[1];
	auto P2 = polygonA[2];

	float z = ((Eb.y - P0.y) * ((P1.x - P0.x) * (P2.z - P0.z) - (P2.x - P0.x) * (P1.z - P0.z)) - 
		(Eb.x - P0.x) * ((P1.y - P0.y) * (P2.z - P0.z) - (P2.y - P0.y) * (P1.z - P0.z)) ) / 
		((P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y)) + P0.z;

	if (Eb.z < z) {
		if (swapped) {
			return 1;
		}
		else {
			return -1;
		}
	}
	else {
		if (swapped) {
			return -1;
		}
		else {
			return 1;
		}
	}
}

// Тип проекции (перспективная или ортографическая)

class Painter
{
public:

	void Draw(Frame& frame)
	{

		float angle = time; // Угол поворота объекта

		auto A = get_points(scale);

		Matrix projection_matrix; // Матрица проектирования

		// Выбор матрицы проектирования
		if (frame.perspective == Perspective::ORTHO) //Ортографическое проектирование 
		{
			projection_matrix = Matrix::Ortho(-2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 1, 140.0f);
		}
		else if (frame.perspective == Perspective::FRUSTUM) // Перспективное проектирование
		{
			projection_matrix = Matrix::Frustum(-0.5 * frame.width / frame.height, 0.5 * frame.width / frame.height, -0.5, 0.5, 1, 140);
		}
		else if (frame.perspective == Perspective::TRIMETRIC) {
			projection_matrix = Matrix::Axonometric(3.14 / 4., 3.14 / 6., -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 0, 140.0f) * Matrix::Translation(0, 0, 1);
		}
		else if (frame.perspective == Perspective::DIMETRIC) {
			projection_matrix = Matrix::Axonometric(0.5152212, 0.45779986, -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 0, 140.0f) * Matrix::Translation(0, 0, 1);
		}
		else if (frame.perspective == Perspective::ISOMETRIC) {
			projection_matrix = Matrix::Axonometric(0.615479708, 3.14 / 4., -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 0, 140.0f) * Matrix::Translation(0, 0, 1);
		}


		Matrix proj_viewport = projection_matrix *                 // Проектирование
			Matrix::Viewport(0, 0, frame.width, frame.height);

		Matrix general_matrix =
			frame.transform *
			proj_viewport; // Преобразование нормализованных координат в оконные

		std::vector<Vector> B(A.size());

		for (int i = 0; i < A.size(); i++)
		{
			B[i] = A[i] * general_matrix;

			// Преобразование однородных координат в обычные
			B[i].x /= B[i].w;
			B[i].y /= B[i].w;
			B[i].z /= B[i].w;
			B[i].w = 1;
		}

		std::vector<std::vector<Vector>> polygons_vals;
		std::vector<int> polygons_indices;
		for (int i = 0; i < polygons.size(); i++) {
			polygons_vals.push_back({
				B[std::get<0>(polygons[i])],
				B[std::get<1>(polygons[i])],
				B[std::get<2>(polygons[i])]
			});
			polygons_indices.push_back(i);
		}

		auto H = std::vector<std::vector<int>>(polygons.size(), std::vector<int>(polygons.size(), 0));
		for (int i = 0; i < polygons.size(); i++) {
			for (int j = 0; j < i; j++) {
				auto res = cmp_triangles(polygons_vals[i], polygons_vals[j]);
				H[i][j] = res;
				H[j][i] = -res;
			}
		}

		while (H.size() != 0) {

			int index_to_del = 0;
			int min_ones = H.size();
			for (int i = 0; i < H.size(); i++) {
				int ones = 0;

				for (int j = 0; j < H[i].size(); j++) {
					if (H[i][j] == 1) {
						ones++;
					}
				}

				if (ones < min_ones) {
					min_ones = ones;
					index_to_del = i;
				}
			}
			
			frame.Triangle(
				polygons_vals[index_to_del][0].x, polygons_vals[index_to_del][0].y, polygons_vals[index_to_del][0].z, polygons_vals[index_to_del][0].w,
				polygons_vals[index_to_del][1].x, polygons_vals[index_to_del][1].y, polygons_vals[index_to_del][1].z, polygons_vals[index_to_del][1].w,
				polygons_vals[index_to_del][2].x, polygons_vals[index_to_del][2].y, polygons_vals[index_to_del][2].z, polygons_vals[index_to_del][2].w,
				materials[polygons_indices[index_to_del]], SHOW_POLYGON);
			
			H.erase(H.begin() + index_to_del);
			for (int i = 0; i < H.size(); i++) {
				H[i].erase(H[i].begin() + index_to_del);
			}

			polygons_vals.erase(polygons_vals.begin() + index_to_del);
			polygons_indices.erase(polygons_indices.begin() + index_to_del);
		}

		/*for (int i = 0; i < polygons.size(); i++) {
			auto pointA = B[std::get<0>(polygons[i])];
			auto pointB = B[std::get<1>(polygons[i])];
			auto pointC = B[std::get<2>(polygons[i])];
			auto polygonColor = materials[i];

			frame.Triangle(pointA.x, pointA.y, pointA.z, pointA.w, pointB.x, pointB.y, pointB.z, pointB.w, pointC.x, pointC.y, pointC.z, pointC.w, polygonColor, SHOW_GRID);
		}*/
	}
};

#endif // PAINTER_H