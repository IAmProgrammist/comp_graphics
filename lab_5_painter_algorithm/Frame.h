#ifndef FRAME_H
#define FRAME_H

#include <math.h>
#include "Matrix.h"
#include "Color.h"
#include "Perspective.h"


#define INSIDE 0 // 0000
#define LEFT 1 // 000
#define RIGHT 2 // 0010
#define BOTTOM 4 // 0100
#define TOP 8 // 1000
#define SHOW_POLYGON 0b01
#define SHOW_GRID    0b10

template<typename TYPE> void swap(TYPE& a, TYPE& b)
{
	TYPE t = a;
	a = b;
	b = t;
}

// Буфер кадра
class Frame 
{
	// Указатель на массив пикселей
	// Буфер кадра будет представлять собой матрицу, которая располагается в памяти в виде непрерывного блока
	PIXEL* pixels;

public:

	// Размеры буфера кадра
	int width, height;
	Matrix transform;
	Perspective perspective;

	Frame(int _width, int _height, Perspective _perspective = Perspective::FRUSTUM, Matrix _transform = Matrix()) :
		width(_width), height(_height), perspective(_perspective), transform(_transform)
	{
		int size = width * height;

		// Создание буфера кадра в виде непрерывной матрицы пикселей
		pixels = new PIXEL[size];

	}

	// Задаёт цвет color пикселю с координатами (x, y)
	void SetPixel(int x, int y, COLOR color)
	{
		PIXEL* pixel = pixels + (size_t)y * width + x; // Находим нужный пиксель в матрице пикселей
		pixel->RED = color.RED;
		pixel->GREEN = color.GREEN;
		pixel->BLUE = color.BLUE;
	}

	// Возвращает цвет пикселя с координатами (x, y)
	COLOR GetPixel(int x, int y) 
	{
		PIXEL* pixel = pixels + (size_t)y * width + x; // Находим нужный пиксель в матрице пикселей
		return COLOR(pixel->RED, pixel->GREEN, pixel->BLUE);
	}
	
	void Triangle(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, COLOR color)
	{
		// Отсортируем точки таким образом, чтобы выполнилось условие: y0 < y1 < y2
		if (y1 < y0)
		{
			swap(x1, x0);
			swap(y1, y0);
			swap(z1, z0);
			swap(w1, w0);
		}
		if (y2 < y1)
		{
			swap(x2, x1);
			swap(y2, y1);
			swap(z2, z1);
			swap(w2, w1);
		}
		if (y1 < y0)
		{
			swap(x1, x0);
			swap(y1, y0);
			swap(z1, z0);
			swap(w1, w0);
		}

		// Определяем номера строк пикселей, в которых располагаются точки треугольника
		int Y0 = (int) (y0 + 0.5f);
		int Y1 = (int) (y1 + 0.5f);
		int Y2 = (int) (y2 + 0.5f);

		// Отсечение невидимой части треугольника
		if (Y0 < 0) Y0 = 0;
		else if (Y0 > height) Y0 = height;

		if (Y1 < 0) Y1 = 0;
		else if (Y1 > height) Y1 = height;
		
		if (Y2 < 0) Y2 = 0;
		else if (Y2 > height) Y2 = height;

		float S = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); // Площадь треугольника
		
		// Рисование верхней части треугольника 
		for (int Y = Y0; Y < Y1; Y++)
		{
			float y = Y + 0.5; // Координата y середины пикселя

			// Вычисление координат граничных пикселей
			int X0 = (int) ((y - y0) / (y1 - y0) * (x1 - x0) + x0 + 0.5f);
			int X1 = (int) ((y - y0) / (y2 - y0) * (x2 - x0) + x0 + 0.5f);

			if (X0 > X1) swap(X0, X1); // Сортировка пикселей
			if (X0 < 0) X0 = 0; // Отсечение невидимых пикселей в строке y
			if (X1 > width) X1 = width;

			for (int X = X0; X < X1; X++)
			{
				double x = X + 0.5; // Координата x середины пикселя

				// Середина пикселя имеет координаты (x, y)
				// Вычислим барицентрические координаты этого пикселя
				float h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
				float h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
				float h2 = ((y0 - y1) * (x - x1) + (x1 - x0) * (y - y1)) / S;

				double ih0 = h0 / w0;
				double ih1 = h1 / w1;
				double ih2 = h2 / w2;

				double NZ = 1.0 / (ih0 + ih1 + ih2);

				h0 = ih0 * NZ;
				h1 = ih1 * NZ;
				h2 = ih2 * NZ;

				float Z = h0 * z0 + h1 * z1 + h2 * z2; // Глубина пикселя

				// Определение глубины точки в экранной системе координат				
				PIXEL* pixel = pixels + (size_t)Y * width + X; // Вычислим адрес пикселя (Y, X) в матрице пикселей pixels
				// pixel->Z - глубина пикселя, которая уже записана в буфер кадра
				// Если текущий пиксель находится ближе того пикселя, который уже записан в буфере кадра
				if (Z > -1 && Z < 1 && Z < pixel->Z)
				{ // то обновляем пиксель в буфере кадра
					//float zmax = 0.8, zmin = 0.1;
					//color = COLOR(255 - (Z - zmin) / (zmax - zmin) * 255, 100, 100);
				    //color = COLOR((2.5 - Z) / 4 * 255, 0, 0);
					pixel->RED = 255 * (1 + Z) / 2;
					pixel->GREEN = 128 * (1 + Z) / 2;
					pixel->BLUE = 0;
					pixel->Z = Z;
				}
			}
		}

		// Рисование нижней части треугольника
		for (int Y = Y1; Y < Y2; Y++)
		{
			float y = Y + 0.5; // Координата y середины пикселя

			// Вычисление координат граничных пикселей
			int X0 = (int)((y - y1) / (y2 - y1) * (x2 - x1) + x1 + 0.5f);
			int X1 = (int)((y - y0) / (y2 - y0) * (x2 - x0) + x0 + 0.5f);

			if (X0 > X1) swap(X0, X1);  // Сортировка пикселей
			if (X0 < 0) X0 = 0; // Отсечение невидимых пикселей в строке y
			if (X1 > width) X1 = width;

			for (int X = X0; X < X1; X++)
			{
				double x = X + 0.5; // Координата x середины пикселя

				// Середина пикселя имеет координаты (x, y)
				// Вычислим барицентрические координаты этого пикселя
				float h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
				float h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
				float h2 = ((y0 - y1) * (x - x1) + (x1 - x0) * (y - y1)) / S;

				float Z = h0 * z0 + h1 * z1 + h2 * z2; // Глубина пикселя

				// Определение глубины точки в экранной системе координат
				// Если текущий пиксель находится ближе того пикселя, который уже записан в буфере кадра
				PIXEL* pixel = pixels + (size_t)Y * width + X;
				if (Z > -1 && Z < 1 && Z < pixel->Z)
				{
					//float zmax = 0.8, zmin = 0.1;
					//color = COLOR(255 - (Z - zmin) / (zmax - zmin) * 255, 100, 100);
					//color = COLOR((2.5 - Z) / 4 * 255, 0, 0);
					pixel->RED = 255 * (1 + Z) / 2;
					pixel->GREEN = 128 * (1 + Z) / 2;
					pixel->BLUE = 0;
					pixel->Z = Z;
				}
			}
		}
	}


	void Quad(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3, COLOR color)
	{
		Triangle(x0, y0, z0, w0, x1, y1, z1, w1, x2, y2, z2, w2, color);
		Triangle(x2, y2, z2, w2, x3, y3, z3, w3, x0, y0, z0, w0, color);
	}


	~Frame(void)
	{
		delete []pixels;
	}

};


#endif // FRAME_H
