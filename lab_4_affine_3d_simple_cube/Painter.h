#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"
#include "Vector.h"
#include "Model.h"

// Время от начала запуска программы
float time = 0;
float scale = 0;
float x_offset = 0;
float y_offset = 0;
float z_offset = -4;
float x_rot = 0;
float y_rot = 0;
float z_rot = 0;
float fig_scale = 2.5;
int draw_mode = 1;
Perspective currentPerspective = static_cast<Perspective>(0);

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
            projection_matrix = Matrix::Axonometric(3.14 / 4., 3.14 / 6., -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 1, 140.0f);
        }
        else if (frame.perspective == Perspective::DIMETRIC) {
            projection_matrix = Matrix::Axonometric(0.5061, 0.45378561, -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 1, 140.0f);
        }
        else if (frame.perspective == Perspective::ISOMETRIC) {
            projection_matrix = Matrix::Axonometric(0.61086523819, 3.14 / 4., -2.0 * frame.width / frame.height, 2.0 * frame.width / frame.height, -2.0, 2.0, 1, 140.0f);
        }

        
        Matrix proj_viewport = projection_matrix *                 // Проектирование
            Matrix::Viewport(0, 0, frame.width, frame.height);

        Matrix general_matrix =
            frame.transform *     // Перенос куба против оси z
            proj_viewport; // Преобразование нормализованных координат в оконные

        std::vector<Vector> B(A.size());
        std::vector<Vector> Axises = {
            {0, 0, 0},
            {5, 0, 0},
            {0, 5, 0},
            {0, 0, 5},
        };

        std::vector<COLOR> AxisesColors = {
            {255, 0, 0},
            {0, 255, 0},
            {0, 0, 255},
        };

        for (int i = 0; i < A.size(); i++)
        {
            B[i] = A[i] * general_matrix;

            // Преобразование однородных координат в обычные
            B[i].x /= B[i].w;
            B[i].y /= B[i].w;
            B[i].z /= B[i].w;
            B[i].w = 1;
        }

        for (int i = 0; i < Axises.size(); i++)
        {
            Axises[i] = Axises[i] * proj_viewport;

            // Преобразование однородных координат в обычные
            Axises[i].x /= Axises[i].w;
            Axises[i].y /= Axises[i].w;
            Axises[i].z /= Axises[i].w;
            Axises[i].w = 1;
        }

        for (int i = 1; i < Axises.size(); i++) {
            frame.DrawLine(Axises[i].x, Axises[i].y, Axises[0].x, Axises[0].y, AxisesColors[i - 1]);
        }

        for (int i = 0; i < polygons.size(); i++) {
            auto pointA = B[std::get<0>(polygons[i])];
            auto pointB = B[std::get<1>(polygons[i])];
            auto pointC = B[std::get<2>(polygons[i])];
            auto polygonColor = materials[i];

            frame.Triangle(pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z, pointC.x, pointC.y, pointC.z, polygonColor, draw_mode);
        }
    }
};

#endif // PAINTER_H