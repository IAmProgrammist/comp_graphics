#ifndef FRAME_H
#define FRAME_H

#include <math.h>
#include <iostream>

// Cтруктура для задания цвета
typedef struct tagCOLOR
{
	unsigned char RED;		// Компонента красного цвета
	unsigned char GREEN;	// Компонента зелёного цвета
	unsigned char BLUE;		// Компонента синего цвета
	unsigned char ALPHA;	// Прозрачность (альфа канал)

	tagCOLOR() : RED(0), GREEN(0), BLUE(0), ALPHA(255) { }
	tagCOLOR(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255) : RED(red), GREEN(green), BLUE(blue), ALPHA(alpha) { }

} COLOR;


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
	COLOR* pixels;

	// Указатели на строки пикселей буфера кадра
	COLOR** matrix;

public:

	// Размеры буфера кадра
	int width, height;

	Frame(int _width, int _height) : width(_width), height(_height)
	{
		int size = width * height;

		// Создание буфера кадра в виде непрерывной матрицы пикселей
		pixels = new COLOR[size];

		// Указатели на строки пикселей запишем в отдельный массив
		matrix = new COLOR* [height];

		// Инициализация массива указателей
		for (int i = 0; i < height; i++)
		{
			matrix[i] = pixels + i * width;
		}
	}


	// Задаёт цвет color пикселю с координатами (x, y)
	void SetPixel(int x, int y, COLOR color)
	{
		matrix[y][x] = color;
	}

	// Возвращает цвет пикселя с координатами (x, y)
	COLOR GetPixel(int x, int y) 
	{
		return matrix[y][x];
	}


	// Рисование окружности
	void Circle(int x0, int y0, int radius, COLOR color)
	{
		int x = 0, y = radius;
		int DSUM = 2 * x * x + 2 * y * y - 2 * radius * radius - 2 * y + 1;
		while(x < y)
		{
			// Если ближе точка (x, y - 1), то смещаемся к ней
			if (DSUM > 0) {
				DSUM -= 4 * y - 4;
				y--;
			}

			// Перенос и отражение вычисленных координат на все октанты окружности 
			SetPixel(x0 + x, y0 + y, color);
			SetPixel(x0 + x, y0 - y, color);
			SetPixel(x0 + y, y0 + x, color);
			SetPixel(x0 + y, y0 - x, color);
			SetPixel(x0 - x, y0 + y, color);
			SetPixel(x0 - x, y0 - y, color);
			SetPixel(x0 - y, y0 + x, color);
			SetPixel(x0 - y, y0 - x, color);
			x++;
			DSUM -= -4 * x - 2;
		}
	}

	inline double getangle(int x, int y) {
		double x1 = 0;
		double y1 = 1;
		double x2 = x;
		double y2 = y;

		double dot = x1 * x2 + y1 * y2;
		double det = x1 * y2 - y1 * x2;
		return atan2(det, dot);
	}

	// Рисование окружности
	void SectorCircle(double dega, double degb, int x0, int y0, int radius, COLOR color)
	{
		int x = 0, y = radius;
		int DSUM = 2 * x * x + 2 * y * y - 2 * radius * radius - 2 * y + 1;
		while (x < y)
		{
			// Если ближе точка (x, y - 1), то смещаемся к ней
			if (DSUM > 0) {
				DSUM -= 4 * y - 4;
				y--;
			}

			// Перенос и отражение вычисленных координат на все октанты окружности 
			double a = getangle(x, y);
			if (a >= dega && a <= degb) SetPixel(x0 + x, y0 + y, color);
			a = getangle(x, -y);
			if (a >= dega && a <= degb) SetPixel(x0 + x, y0 - y, color);
			a = getangle(y, x);
			if (a >= dega && a <= degb) SetPixel(x0 + y, y0 + x, color);
			a = getangle(y, -x);
			if (a >= dega && a <= degb) SetPixel(x0 + y, y0 - x, color);
			a = getangle(-x, y);
			if (a >= dega && a <= degb) SetPixel(x0 - x, y0 + y, color);
			a = getangle(-x, -y);
			if (a >= dega && a <= degb) SetPixel(x0 - x, y0 - y, color);
			a = getangle(-y, x);
			if (a >= dega && a <= degb) SetPixel(x0 - y, y0 + x, color);
			a = getangle(-y, -x);
			if (a >= dega && a <= degb) SetPixel(x0 - y, y0 - x, color);

			x++;
			DSUM -= -4 * x - 2;
		}
	}


	// Рисование отрезка
	void DrawLine(int x1, int y1, int x2, int y2, COLOR color)
	{
		int dy = y2 - y1, dx = x2 - x1;
		if (dx == 0 && dy == 0)
		{
			matrix[y1][x1] = color;
			return;
		}

		if (abs(dx) > abs(dy))
		{
			if (x2 < x1)
			{
				// Обмен местами точек (x1, y1) и (x2, y2)
				swap(x1, x2);
				swap(y1, y2);
				dx = -dx; dy = -dy;
			}
			
			int y = y1;
			int sign_factor = dy < 0 ? 1 : -1;
			int sumd = - 2 * (y - y1) * dx + sign_factor * dx;
			for (int x = x1; x <= x2; x++)
			{
				if (sign_factor * sumd < 0) {
					y -= sign_factor;
					sumd += sign_factor * dx;
				}

				sumd += dy;

				matrix[y][x] = color;
			}
		}
		else
		{ 
			if (y2 < y1)
			{
				// Обмен местами точек (x1, y1) и (x2, y2)
				swap(x1, x2);
				swap(y1, y2);
				dx = -dx; dy = -dy;
			}

			int x = x1;
			int sign_factor = dx > 0 ? 1 : -1;
			int sumd = 2 * (x - x1) * dy + sign_factor * dy;
			for (int y = y1; y <= y2; y++)
			{
				if (sign_factor * sumd < 0) {
					x += sign_factor;
					sumd += sign_factor * dy;
				}

				sumd -= dx;

				matrix[y][x] = color;
			}
		}
	}


	~Frame(void)
	{
		delete []pixels;
		delete []matrix;
	}

};


#endif // FRAME_H
