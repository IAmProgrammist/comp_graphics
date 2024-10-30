#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"
#include "Vector.h"
#include "Matrix.h"
#include "Model.h"

// Время от начала запуска программы
float time = 0;
float scale = 0;

// Тип проекции (перспективная или ортографическая)
bool bFrustum = true;

class Painter
{
public:

    void Draw(Frame& frame)
    {

        float angle = time; // Угол поворота объекта

        auto A = get_points(scale);

        Matrix projection_matrix; // Матрица проектирования

        // Выбор матрицы проектирования
        if (!bFrustum || frame.mode != 3) // Перспективное проектирование
        {
            projection_matrix = Matrix::Ortho(-2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 1.0f, 20.0f);
        }
        else // Ортографическое проектирование
        {
            projection_matrix = Matrix::Frustum(-0.5 * frame.width / frame.height, 0.5 * frame.width / frame.height, -0.5, 0.5, 1.0, 20);
        }

        
        Matrix general_matrix;
        if (frame.mode == 0) {
            general_matrix =
                Matrix::Scale(2, 2, 2) *
                Matrix::Translation(0, 0, -4) *     // Перенос куба против оси z
                projection_matrix *                 // Проектирование
                Matrix::Viewport(0, 0, frame.width, frame.height); // Преобразование нормализованных координат в оконные
        } else if (frame.mode == 1) {
            general_matrix =
                Matrix::Scale(2, 2, 2) *
                Matrix::RotationY(3.14 / 2) *
                Matrix::Translation(0, 0, -4) *     // Перенос куба против оси z
                projection_matrix *                 // Проектирование
                Matrix::Viewport(0, 0, frame.width, frame.height); // Преобразование нормализованных координат в оконные
        }
        else if (frame.mode == 2) {
            general_matrix =
                Matrix::Scale(2, 2, 2) *
                Matrix::RotationX(3.14 / 2) *
                Matrix::Translation(0, 0, -4) *     // Перенос куба против оси z
                projection_matrix *                 // Проектирование
                Matrix::Viewport(0, 0, frame.width, frame.height); // Преобразование нормализованных координат в оконные
        }
        else {
            general_matrix =
                Matrix::Scale(2, 2, 2) *
                Matrix::RotationX(angle / 2) *      // Поворот куба вокруг оси x
                Matrix::RotationY(angle / 4) *     // Поворот куба вокруг оси y
                Matrix::Translation(0, 0, -4) *     // Перенос куба против оси z
                projection_matrix *                 // Проектирование
                Matrix::Viewport(0, 0, frame.width, frame.height); // Преобразование нормализованных координат в оконные
        }

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

        for (int i = 0; i < polygons.size(); i++) {
            auto pointA = B[std::get<0>(polygons[i])];
            auto pointB = B[std::get<1>(polygons[i])];
            auto pointC = B[std::get<2>(polygons[i])];
            auto polygonColor = materials[i];

            frame.Triangle(pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z, pointC.x, pointC.y, pointC.z, polygonColor);
        }
    }
};

#endif // PAINTER_H