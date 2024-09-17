#ifndef FRAME_H
#define FRAME_H

#include <math.h>

// Cтруктура для задания цвета
typedef struct tagCOLOR
{
	unsigned char RED;		// Компонента красного цвета
	unsigned char GREEN;	// Компонента зелёного цвета
	unsigned char BLUE;		// Компонента синего цвета
	unsigned char ALPHA;	// Прозрачность (альфа канал)

	tagCOLOR() : RED(0), GREEN(0), BLUE(0), ALPHA(255) { }

	tagCOLOR(int red, int green, int blue, int alpha = 255)
		: RED(red), GREEN(green), BLUE(blue), ALPHA(alpha)
	{
		if (red < 0) RED = 0;
		else if (red > 255) RED = 255;
		if (green < 0) GREEN = 0;
		else if (green > 255) GREEN = 255;
		if (blue < 0) BLUE = 0;
		else if (blue > 255) BLUE = 255;
		if (alpha < 0) ALPHA = 0;
		else if (alpha > 255) ALPHA = 255;
	}

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
		matrix = new COLOR * [height];

		// Инициализация массива указателей
		for (int i = 0; i < height; i++)
		{
			matrix[i] = pixels + (size_t)i * width;
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


	void Triangle(float x0, float y0, float x1, float y1, float x2, float y2, COLOR color)
	{
		// Отсортируем точки таким образом, чтобы выполнилось условие: y0 < y1 < y2
		if (y1 < y0)
		{
			swap(y1, y0);
			swap(x1, x0);
		}
		if (y2 < y1)
		{
			swap(y2, y1);
			swap(x2, x1);
		}
		if (y1 < y0)
		{
			swap(y1, y0);
			swap(x1, x0);
		}

		// Определяем номера строк пикселей, в которых располагаются точки треугольника
		int Y0 = (int)(y0 + 0.5f);
		int Y1 = (int)(y1 + 0.5f);
		int Y2 = (int)(y2 + 0.5f);

		// Отсечение невидимой части треугольника
		if (Y0 < 0) Y0 = 0;
		else if (Y0 >= height) Y0 = height;

		if (Y1 < 0) Y1 = 0;
		else if (Y1 >= height) Y1 = height;

		if (Y2 < 0) Y2 = 0;
		else if (Y2 >= height) Y2 = height;


		double rawX0 = (Y0 + 0.5f - y0) / (y1 - y0) * (x1 - x0) + x0;
		double rawX1 = (Y0 + 0.5f - y0) / (y2 - y0) * (x2 - x0) + x0;
		bool should_swap = rawX0 > rawX1;
		int X0, X1;
		// Рисование верхней части треугольника
		for (float y = Y0 + 0.5f; y < Y1; y++)
		{

			X0 = rawX0;
			X1 = rawX1;
			if (should_swap) swap(X0, X1);
			if (X0 < 0) X0 = 0;
			if (X1 > width) X1 = width;

			for (int x = X0; x < X1; x++)
			{
				// f(x + 0.5, y)
				SetPixel(x, y, color);
			}

			rawX0 += (x1 - x0) / (y1 - y0);
			rawX1 += (x2 - x0) / (y2 - y0);
		}

		rawX0 = (Y1 + 0.5f - y1) / (y2 - y1) * (x2 - x1) + x1;
		rawX1 = (Y1 + 0.5f - y0) / (y2 - y0) * (x2 - x0) + x0;
		should_swap = rawX0 > rawX1;
		// Рисование нижней части треугольника
		for (float y = Y1 + 0.5f; y < Y2; y++)
		{
			X0 = rawX0;
			X1 = rawX1;

			if (should_swap) swap(X0, X1);
			if (X0 < 0) X0 = 0;
			if (X1 > width) X1 = width;

			for (int x = X0; x < X1; x++)
			{
				// f(x + 0.5, y)
				SetPixel(x, y, color);
			}

			rawX0 += (x2 - x1) / (y2 - y1);
			rawX1 += (x2 - x0) / (y2 - y0);
		}

	}

	template <class InterpolatorClass>
	void Triangle(float x0, float y0, float x1, float y1, float x2, float y2, InterpolatorClass& Interpolator)
	{
		// Отсортируем точки таким образом, чтобы выполнилось условие: y0 < y1 < y2
		if (y1 < y0)
		{
			swap(y1, y0);
			swap(x1, x0);
		}
		if (y2 < y1)
		{
			swap(y2, y1);
			swap(x2, x1);
		}
		if (y1 < y0)
		{
			swap(y1, y0);
			swap(x1, x0);
		}

		// Определяем номера строк пикселей, в которых располагаются точки треугольника
		int Y0 = (int)(y0 + 0.5f);
		int Y1 = (int)(y1 + 0.5f);
		int Y2 = (int)(y2 + 0.5f);

		// Отсечение невидимой части треугольника
		if (Y0 < 0) Y0 = 0;
		else if (Y0 >= height) Y0 = height;

		if (Y1 < 0) Y1 = 0;
		else if (Y1 >= height) Y1 = height;

		if (Y2 < 0) Y2 = 0;
		else if (Y2 >= height) Y2 = height;

		double rawX0 = (Y0 + 0.5f - y0) / (y1 - y0) * (x1 - x0) + x0;
		double rawX1 = (Y0 + 0.5f - y0) / (y2 - y0) * (x2 - x0) + x0;
		bool should_swap = rawX0 > rawX1;
		int X0, X1;


		// Рисование верхней части треугольника
		for (float y = Y0 + 0.5f; y < Y1; y++)
		{
			X0 = rawX0;
			X1 = rawX1;
			if (should_swap) swap(X0, X1);
			if (X0 < 0) X0 = 0;
			if (X1 > width) X1 = width;

			for (int x = X0; x <= X1; x++)
			{
				// f(x + 0.5, y)
				COLOR color = Interpolator.color(x + 0.5f, y);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[(int)y][x]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x, y, color);
			}

			rawX0 += (x1 - x0) / (y1 - y0);
			rawX1 += (x2 - x0) / (y2 - y0);
		}

		rawX0 = (Y1 + 0.5f - y1) / (y2 - y1) * (x2 - x1) + x1;
		rawX1 = (Y1 + 0.5f - y0) / (y2 - y0) * (x2 - x0) + x0;
		should_swap = rawX0 > rawX1;

		// Рисование нижней части треугольника
		for (float y = Y1 + 0.5f; y < Y2; y++)
		{
			X0 = rawX0;
			X1 = rawX1;

			if (should_swap) swap(X0, X1);
			if (X0 < 0) X0 = 0;
			if (X1 > width) X1 = width;

			for (int x = X0; x <= X1; x++)
			{
				// f(x + 0.5, y)
				COLOR color = Interpolator.color(x + 0.5f, y);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[(int)y][x]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x, y, color);
			}

			rawX0 += (x2 - x1) / (y2 - y1);
			rawX1 += (x2 - x0) / (y2 - y0);
		}

	}

	void Circle(int x0, int y0, int radius, COLOR color)
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

	template <class InterpolatorClass>
	void Circle(int x0, int y0, int radius, InterpolatorClass& Interpolator)
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
			for (int X0 = -x; X0 <= x; X0++) {
				COLOR color = Interpolator.color(x0 + X0 + 0.5f, y0 + y);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[y0 + y][x0 + X0]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x0 + X0, y0 + y, color);

				color = Interpolator.color(x0 + X0 + 0.5f, y0 - y);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[y0 - y][x0 + X0]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x0 + X0, y0 - y, color);
			}

			for (int X0 = -y; X0 <= y; X0++) {
				COLOR color = Interpolator.color(x0 + X0 + 0.5f, y0 + x);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[y0 + x][x0 + X0]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x0 + X0, y0 + x, color);

				color = Interpolator.color(x0 + X0 + 0.5f, y0 - x);

				// Для рисования полупрозрачных фигур будем использовать альфа-смешивание
				if (color.ALPHA < 255)
				{
					COLOR written = matrix[y0 - x][x0 + X0]; // Уже записанное в буфере кадра значение цвета, т.е. цвет фона
					float a = color.ALPHA / 255.0f, b = 1 - a;
					color.RED = color.RED * a + written.RED * b;
					color.GREEN = color.GREEN * a + written.GREEN * b;
					color.BLUE = color.BLUE * a + written.BLUE * b;
				}

				SetPixel(x0 + X0, y0 - x, color);
			}

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
			int sumd = -2 * (y - y1) * dx + sign_factor * dx;
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
		delete[]pixels;
		delete[]matrix;
	}

};


#endif // FRAME_H
