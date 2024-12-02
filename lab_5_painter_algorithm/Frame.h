#ifndef FRAME_H
#define FRAME_H

#include <math.h>
#include <algorithm>
#include "Matrix.h"
#include "Color.h"
#include "Perspective.h"
#include "Shaders.h"


#define INSIDE 0 // 0000
#define LEFT 1 // 000
#define RIGHT 2 // 0010
#define BOTTOM 4 // 0100
#define TOP 8 // 1000
#define SHOW_POLYGON 0b01
#define SHOW_GRID    0b10



// Cтруктура для задания цвета
typedef struct HSVCOLOR
{
	double H;		// Компонента красного цвета
	double S;	// Компонента зелёного цвета
	double V;		// Компонента синего цвета
	unsigned char ALPHA;	// Прозрачность (альфа канал)

	HSVCOLOR() : H(0), S(0), V(0), ALPHA(255) { }

	HSVCOLOR(double hue, double saturation, double value, int alpha = 255)
		: H(hue), S(saturation), V(value), ALPHA(alpha)
	{
		if (hue < 0) H = 0;
		else if (hue > 360) H = 360;
		if (saturation < 0) S = 0;
		else if (saturation > 1) S = 1;
		if (value < 0) value = 0;
		else if (value > 1) V = 1;
		if (alpha < 0) ALPHA = 0;
		else if (alpha > 255) ALPHA = 255;
	}

	COLOR convertToRgb() {
		int hi = int(floor(H / 60)) % 6;
		double f = H / 60 - floor(H / 60);
		int copyV = V * 255;
		int v = (int)(copyV);
		int p = (int)(copyV * (1 - S));
		int q = (int)(copyV * (1 - f * S));
		int t = (int)(copyV * (1 - (1 - f) * S));
		if (hi == 0)
			return { v, t, p, ALPHA };
		if (hi == 1)
			return { q, v, p, ALPHA };
		else if (hi == 2)
			return { p, v, t, ALPHA };
		else if (hi == 3)
			return { p, q, v, ALPHA };
		else if (hi == 4)
			return { t, p, v, ALPHA };
		return { v, p, q, ALPHA };
	}

} HSVCOLOR;

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

	int getCohenSutherland(int x, int y) {
		int y_max = height - 1;
		int y_min = 0;
		int x_max = width - 1;
		int x_min = 0;
		// initialized as being inside
		int code = INSIDE;

		if (x < x_min) // to the left of rectangle
			code |= LEFT;
		else if (x > x_max) // to the right of rectangle
			code |= RIGHT;
		if (y < y_min) // below the rectangle
			code |= BOTTOM;
		else if (y > y_max) // above the rectangle
			code |= TOP;

		return code;
	}

	void cohenSutherlandClip(int x1, int y1,
		int x2, int y2, bool* accepted,
		int* resX1, int* resY1, int* resX2, int* resY2)
	{
		int y_max = height - 1;
		int y_min = 0;
		int x_max = width - 1;
		int x_min = 0;

		int code1 = getCohenSutherland(x1, y1);
		int code2 = getCohenSutherland(x2, y2);

		bool accept = false;

		while (true) {
			if ((code1 == 0) && (code2 == 0)) {
				accept = true;
				break;
			}
			else if (code1 & code2) {
				break;
			}
			else {
				int code_out;
				int x, y;

				if (code1 != 0)
					code_out = code1;
				else
					code_out = code2;

				if (code_out & TOP) {
					x = x1 + (x2 - x1) * (y_max - y1) / (y2 - y1);
					y = y_max;
				}
				else if (code_out & BOTTOM) {
					x = x1 + (x2 - x1) * (y_min - y1) / (y2 - y1);
					y = y_min;
				}
				else if (code_out & RIGHT) {
					y = y1 + (y2 - y1) * (x_max - x1) / (x2 - x1);
					x = x_max;
				}
				else if (code_out & LEFT) {
					y = y1 + (y2 - y1) * (x_min - x1) / (x2 - x1);
					x = x_min;
				}

				if (code_out == code1) {
					x1 = x;
					y1 = y;
					code1 = getCohenSutherland(x1, y1);
				}
				else {
					x2 = x;
					y2 = y;
					code2 = getCohenSutherland(x2, y2);
				}
			}
		}
		if (accept) {
			*accepted = true;
			*resX1 = x1;
			*resY1 = y1;
			*resX2 = x2;
			*resY2 = y2;
		}
		else {
			*accepted = false;
		}
	}

	// Рисование отрезка
	void DrawLine(int x1, int y1, int x2, int y2, COLOR color)
	{
		bool shouldDraw = false;
		int resX1, resY1, resX2, resY2;
		cohenSutherlandClip(x1, y1, x2, y2, &shouldDraw, &resX1, &resY1, &resX2, &resY2);
		if (!shouldDraw) return;
		x1 = resX1;
		y1 = resY1;
		x2 = resX2;
		y2 = resY2;

		PIXEL* pixel;
		int dy = y2 - y1, dx = x2 - x1;
		if (dx == 0 && dy == 0)
		{
			pixel = pixels + (size_t)y1 * width + x1;
			pixel->RED = color.RED;
			pixel->GREEN = color.GREEN;
			pixel->BLUE = color.BLUE;
			pixel->Z = -1;
			return;
		}

		if (abs(dx) > abs(dy))
		{
			if (x2 < x1)
			{
				// Обмен местами точек (x1, y1) и (x2, y2)
				std::swap(x1, x2);
				std::swap(y1, y2);
				dx = -dx; dy = -dy;
			}

			int y = y1;
			int sign_factor = dy < 0 ? 1 : -1;
			int sumd = -2 * (y - y1) * dx + sign_factor * dx;
			for (int x = x1; x <= x2; x++)
			{
				if (sign_factor * sumd < 0) {
					y -= sign_factor;
					sumd += sign_factor * dx;
				}

				sumd += dy;

				pixel = pixels + (size_t)y * width + x;
				pixel->RED = color.RED;
				pixel->GREEN = color.GREEN;
				pixel->BLUE = color.BLUE;
				pixel->Z = -1;
			}
		}
		else
		{
			if (y2 < y1)
			{
				// Обмен местами точек (x1, y1) и (x2, y2)
				std::swap(x1, x2);
				std::swap(y1, y2);
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

				pixel = pixels + (size_t)y * width + x;
				pixel->RED = color.RED;
				pixel->GREEN = color.GREEN;
				pixel->BLUE = color.BLUE;
				pixel->Z = -1.1;
			}
		}
	}
	
	void Triangle(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, BaseShader* shader, char drawMode = SHOW_GRID | SHOW_POLYGON, COLOR gridColor = { 255, 255, 255, 0 })
	{
		if (!drawMode) return;

		if (drawMode & SHOW_GRID) {
			DrawLine(x0, y0, x1, y1, gridColor);
			DrawLine(x1, y1, x2, y2, gridColor);
			DrawLine(x0, y0, x2, y2, gridColor);
		}

		if (!(drawMode & SHOW_POLYGON))
			return;

		int swap_amount = 0;

		// Отсортируем точки таким образом, чтобы выполнилось условие: y0 < y1 < y2
		if (y1 < y0)
		{
			std::swap(x1, x0);
			std::swap(y1, y0);
			std::swap(z1, z0);
			std::swap(w1, w0);
			swap_amount++;
		}
		if (y2 < y1)
		{
			std::swap(x2, x1);
			std::swap(y2, y1);
			std::swap(z2, z1);
			std::swap(w2, w1);
			swap_amount++;
		}
		if (y1 < y0)
		{
			std::swap(x1, x0);
			std::swap(y1, y0);
			std::swap(z1, z0);
			std::swap(w1, w0);
			swap_amount++;
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

		double S = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); // Площадь треугольника
		
		// Рисование верхней части треугольника 
		for (int Y = Y0; Y < Y1; Y++)
		{
			double y = Y + 0.5; // Координата y середины пикселя

			// Вычисление координат граничных пикселей
			int X0 = (int) ((y - y0) / (y1 - y0) * (x1 - x0) + x0 + 0.5f);
			int X1 = (int) ((y - y0) / (y2 - y0) * (x2 - x0) + x0 + 0.5f);

			if (X0 > X1) std::swap(X0, X1); // Сортировка пикселей
			if (X0 < 0) X0 = 0; // Отсечение невидимых пикселей в строке y
			if (X1 > width) X1 = width;

			for (int X = X0; X < X1; X++)
			{
				double x = X + 0.5; // Координата x середины пикселя

				// Середина пикселя имеет координаты (x, y)
				// Вычислим барицентрические координаты этого пикселя
				double h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
				double h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
				double h2 = ((y0 - y1) * (x - x1) + (x1 - x0) * (y - y1)) / S;

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
				if (Z > -1 && Z < 1/* && Z < pixel->Z*/)
				{ // то обновляем пиксель в буфере кадра
					//float zmax = 0.8, zmin = 0.1;
					//color = COLOR(255 - (Z - zmin) / (zmax - zmin) * 255, 100, 100);
				    //color = COLOR((2.5 - Z) / 4 * 255, 0, 0);
					if (swap_amount > 2) {
						std::swap(h0, h1);
					}
					if (swap_amount > 1) {
						std::swap(h1, h2);
					}
					if (swap_amount > 0) {
						std::swap(h0, h1);
					}


					auto color = shader->main(shader->getVertexData(h0, h1, h2));
					pixel->RED += (color.ALPHA / 255.0) * (color.RED - pixel->RED);
					pixel->GREEN += (color.ALPHA / 255.0) * (color.GREEN - pixel->GREEN);
					pixel->BLUE += (color.ALPHA / 255.0) * (color.BLUE - pixel->BLUE);
					pixel->Z = Z;
				}
			}
		}

		// Рисование нижней части треугольника
		for (int Y = Y1; Y < Y2; Y++)
		{
			double y = Y + 0.5; // Координата y середины пикселя

			// Вычисление координат граничных пикселей
			int X0 = (int)((y - y1) / (y2 - y1) * (x2 - x1) + x1 + 0.5f);
			int X1 = (int)((y - y0) / (y2 - y0) * (x2 - x0) + x0 + 0.5f);

			if (X0 > X1) std::swap(X0, X1);  // Сортировка пикселей
			if (X0 < 0) X0 = 0; // Отсечение невидимых пикселей в строке y
			if (X1 > width) X1 = width;

			for (int X = X0; X < X1; X++)
			{
				double x = X + 0.5; // Координата x середины пикселя

				// Середина пикселя имеет координаты (x, y)
				// Вычислим барицентрические координаты этого пикселя
				double h0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / S;
				double h1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / S;
				double h2 = ((y0 - y1) * (x - x1) + (x1 - x0) * (y - y1)) / S;

				double ih0 = h0 / w0;
				double ih1 = h1 / w1;
				double ih2 = h2 / w2;

				double NZ = 1.0 / (ih0 + ih1 + ih2);

				h0 = ih0 * NZ;
				h1 = ih1 * NZ;
				h2 = ih2 * NZ;

				float Z = h0 * z0 + h1 * z1 + h2 * z2; // Глубина пикселя

				// Определение глубины точки в экранной системе координат
				// Если текущий пиксель находится ближе того пикселя, который уже записан в буфере кадра
				PIXEL* pixel = pixels + (size_t)Y * width + X;
				if (Z > -1 && Z < 1/* && Z < pixel->Z*/)
				{
					if (swap_amount > 2) {
						std::swap(h0, h1);
					}
					if (swap_amount > 1) {
						std::swap(h1, h2);
					}
					if (swap_amount > 0) {
						std::swap(h0, h1);
					}
					//float zmax = 0.8, zmin = 0.1;
					//color = COLOR(255 - (Z - zmin) / (zmax - zmin) * 255, 100, 100);
					//color = COLOR((2.5 - Z) / 4 * 255, 0, 0);
					auto color = shader->main(shader->getVertexData(h0, h1, h2));
					pixel->RED += (color.ALPHA / 255.0) * (color.RED - pixel->RED);
					pixel->GREEN += (color.ALPHA / 255.0) * (color.GREEN - pixel->GREEN);
					pixel->BLUE += (color.ALPHA / 255.0) * (color.BLUE - pixel->BLUE);
					pixel->Z = Z;
				}
			}
		}
	}


	void Quad(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3, BaseShader* shader)
	{
		Triangle(x0, y0, z0, w0, x1, y1, z1, w1, x2, y2, z2, w2, shader);
		Triangle(x2, y2, z2, w2, x3, y3, z3, w3, x0, y0, z0, w0, shader);
	}


	~Frame(void)
	{
		delete []pixels;
	}

};


#endif // FRAME_H
