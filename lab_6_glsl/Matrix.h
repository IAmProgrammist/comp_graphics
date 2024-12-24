#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

class Matrix
{

public:
 float M[4][4];   
    // По умолчанию матрица инициализируется как единичная
    Matrix() : M {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 } }
    {}
   
    // Конструктор, который инициализирует матрицу M поэлементно значениями аргументов
    Matrix( float A00, float A01, float A02, float A03,
            float A10, float A11, float A12, float A13,
            float A20, float A21, float A22, float A23,
            float A30, float A31, float A32, float A33) : M {
                { A00, A01, A02, A03 },
                { A10, A11, A12, A13 },
                { A20, A21, A22, A23 },
                { A30, A31, A32, A33 } }
    {}

    // Конструктор, который инициализирует матрицу M двумерным массивом A
    Matrix(const float A[4][4]) : M {
        { A[0][0], A[0][1], A[0][2], A[0][3] },
        { A[1][0], A[1][1], A[1][2], A[1][3] },
        { A[2][0], A[2][1], A[2][2], A[2][3] },
        { A[3][0], A[3][1], A[3][2], A[3][3] } }
    {}

    Matrix operator * (const Matrix& A) const
    {
        Matrix R;
        char i, j;
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                R.M[i][j] = M[i][0] * A.M[0][j] + M[i][1] * A.M[1][j] + M[i][2] * A.M[2][j] + M[i][3] * A.M[3][j];
        return R;
    }

   static Matrix RotationX(float Angle)
   {
     Matrix R;
     float cosA = cosf(Angle);
     float sinA = sinf(Angle);
     R.M[0][0] = 1; R.M[0][1] = 0;    R.M[0][2] = 0;    R.M[0][3] = 0;
     R.M[1][0] = 0; R.M[1][1] = cosA; R.M[1][2] = sinA; R.M[1][3] = 0;
     R.M[2][0] = 0; R.M[2][1] =-sinA; R.M[2][2] = cosA; R.M[2][3] = 0;
     R.M[3][0] = 0; R.M[3][1] = 0;    R.M[3][2] = 0;    R.M[3][3] = 1;
     return R;
   }

   static Matrix RotationY(float Angle)
   {
     Matrix R;
     float cosA = cosf(Angle);
     float sinA = sinf(Angle);
     R.M[0][0] = cosA; R.M[0][1] = 0; R.M[0][2] =-sinA; R.M[0][3] = 0;
     R.M[1][0] = 0;    R.M[1][1] = 1; R.M[1][2] = 0;    R.M[1][3] = 0;
     R.M[2][0] = sinA; R.M[2][1] = 0; R.M[2][2] = cosA; R.M[2][3] = 0;
     R.M[3][0] = 0;    R.M[3][1] = 0; R.M[3][2] = 0;    R.M[3][3] = 1;
     return R;
   }
   
   static Matrix RotationZ(float Angle)
   {
     Matrix R;
     float cosA = cosf(Angle);
     float sinA = sinf(Angle);
     R.M[0][0] = cosA; R.M[0][1] = sinA; R.M[0][2] = 0; R.M[0][3] = 0;
     R.M[1][0] =-sinA; R.M[1][1] = cosA; R.M[1][2] = 0; R.M[1][3] = 0;
     R.M[2][0] = 0;    R.M[2][1] = 0;    R.M[2][2] = 1; R.M[2][3] = 0;
     R.M[3][0] = 0;    R.M[3][1] = 0;    R.M[3][2] = 0; R.M[3][3] = 1;
     return R;
   }

   static Matrix Translation(float dx, float dy, float dz)
   {
     Matrix R;
     R.M[0][0] = 1;  R.M[0][1] = 0;  R.M[0][2] = 0;  R.M[0][3] = 0;
     R.M[1][0] = 0;  R.M[1][1] = 1;  R.M[1][2] = 0;  R.M[1][3] = 0;
     R.M[2][0] = 0;  R.M[2][1] = 0;  R.M[2][2] = 1;  R.M[2][3] = 0;
     R.M[3][0] = dx; R.M[3][1] = dy; R.M[3][2] = dz; R.M[3][3] = 1;
     return R;
   }

   static Matrix Scale(float kx, float ky, float kz)
   {
       Matrix R;
       R.M[0][0] = kx;  R.M[0][1] = 0;   R.M[0][2] = 0;   R.M[0][3] = 0;
       R.M[1][0] = 0;   R.M[1][1] = ky;  R.M[1][2] = 0;   R.M[1][3] = 0;
       R.M[2][0] = 0;   R.M[2][1] = 0;   R.M[2][2] = kz;  R.M[2][3] = 0;
       R.M[3][0] = 0;   R.M[3][1] = 0;   R.M[3][2] = 0;   R.M[3][3] = 1;
       return R;
   }

  
   // Матрица для преобразования мировых координат в экранные координаты области (порта) вывода
   // (X0, Y0) - экранные координаты левого нижнего угла области (порта) вывода
   // width - ширина порта вывода (в пикселях)
   // height - высота порта вывода (в пикселях)
   // nearPlane - координата z ближней плоскости
   // farPlane - координата z дальней плоскости
   static Matrix Viewport(float X0, float Y0, float width, float height, float nearPlane = -1.0f, float farPlane = 1.0f)
   {
       return Matrix(
          width / 2,               0,                     0,                     0, 
              0,               height / 2,                0,                     0,
              0,                   0,          (farPlane - nearPlane) / 2,       0,
         X0 + width / 2,    Y0 + height / 2,   (farPlane + nearPlane) / 2,       1);
   }


   // Матрица перспективного преобразования
   // verticalAngle - вертикальный угол обзора камеры (в градусах)
   // aspectRatio - соотношение сторон области вывода, которое определяет поле зрения вдоль оси абсцисс
   // Оно обязательно должно быть равно отношению ширины окна к его высоте
   // zNear > 0 - расстояние от наблюдателя до ближней плоскости отсечения
   // zFar > 0 - расстояние от наблюдателя до дальней плоскости отсечения
   static Matrix Perspective(float verticalAngle, float aspectRatio, float zNear, float zFar)
   {
       const float pi = 3.1415926536f;
       float ctg = 1 / tanf(verticalAngle / 2 * pi / 180);
       float dz = zFar - zNear;
       return Matrix( 
          ctg / aspectRatio,         0,                0,                    0,
                 0,                 ctg,               0,                    0,
                 0,                  0,       -(zFar + zNear) / dz,         -1,
                 0,                  0,      -2 * zFar * zNear / dz,         0);
   }


   static Matrix Frustum(float left, float right, float bottom, float top, float nearPlane, float farPlane)
   {
       float width = right - left;
       float invheight = top - bottom;
       float clip = farPlane - nearPlane;
       Matrix R;
       R.M[0][0] = 2.0f * nearPlane / width;
       R.M[1][0] = 0.0f;
       R.M[2][0] = (left + right) / width;
       R.M[3][0] = 0.0f;
       R.M[0][1] = 0.0f;
       R.M[1][1] = 2.0f * nearPlane / invheight;
       R.M[2][1] = (top + bottom) / invheight;
       R.M[3][1] = 0.0f;
       R.M[0][2] = 0.0f;
       R.M[1][2] = 0.0f;
       R.M[2][2] = -(nearPlane + farPlane) / clip;
       R.M[3][2] = -2.0f * nearPlane * farPlane / clip;
       R.M[0][3] = 0.0f;
       R.M[1][3] = 0.0f;
       R.M[2][3] = -1.0f;
       R.M[3][3] = 0.0f;
       return R;
   }


   static Matrix Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
   {
       float width = right - left;
       float invheight = top - bottom;
       float clip = farPlane - nearPlane;
       Matrix R;
       R.M[0][0] = 2.0f / width;
       R.M[1][0] = 0.0f;
       R.M[2][0] = 0.0f;
       R.M[3][0] = -(left + right) / width;
       R.M[0][1] = 0.0f;
       R.M[1][1] = 2.0f / invheight;
       R.M[2][1] = 0.0f;
       R.M[3][1] = -(top + bottom) / invheight;
       R.M[0][2] = 0.0f;
       R.M[1][2] = 0.0f;
       R.M[2][2] = -2.0f / clip;
       R.M[3][2] = -(nearPlane + farPlane) / clip;
       R.M[0][3] = 0.0f;
       R.M[1][3] = 0.0f;
       R.M[2][3] = 0.0f;
       R.M[3][3] = 1.0f;
       return R;
   }

   friend class Vector;
};



#endif MATRIX_H