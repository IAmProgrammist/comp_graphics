#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"


// Угол поворота фигуры
float global_angle = 0;

// Координаты последнего пикселя, который выбрал пользователь 
struct
{
	int X, Y;
} global_clicked_pixel = {-1, -1};


class Painter
{
public:

	void Draw(Frame& frame)
	{
		// Шахматная текстура
		for (int y = 0; y < frame.height; y++)
			for (int x = 0; x < frame.width; x++)
			{
				if ((x + y) % 2 == 0)
					frame.SetPixel(x, y, { 230, 255, 230 });	// Золотистый цвет
					//frame.SetPixel(x, y, { 217, 168, 14 });	
				else
					frame.SetPixel(x, y, { 200, 200, 200 }); // Чёрный цвет
				//frame.SetPixel(x, y, { 255, 255, 255 }); // Белый цвет
			}


		int W = frame.width, H = frame.height;
		// Размер рисунка возьмём меньше (7 / 8), чтобы он не касался границ экрана 
		float a = 7.0f / 8 * ((W < H) ? W - 1 : H - 1) / sqrt(2);
		if (a < 1) return; // Если окно очень маленькое, то ничего не рисуем
		float angle = global_angle; // Угол поворота
		a = a / 2;
		
		// Инициализируем исходные координаты центра и вершин квадрата
		struct
		{
			float x;
			float y;
		} C = {W / 2, H / 2}, A[4] = { { C.x + a, C.y + a}, {C.x + a, C.y - a}, {C.x - a, C.y - a}, {C.x - a, C.y + a} };
	

		// Поворачиваем все вершины квадрата вокруг точки C на угол angle
		for (int i = 0; i < 4; i++)
		{
			float xi = A[i].x, yi = A[i].y;
			A[i].x = (xi - C.x) * cos(angle) - (yi - C.y) * sin(angle) + C.x;
			A[i].y = (xi - C.x) * sin(angle) + (yi - C.y) * cos(angle) + C.y;
		}

		// Рисуем стороны квадрата
		for (int i = 0; i < 4; i++)
		{
			int i2 = (i + 1) % 4;
			frame.DrawLine( // Добавляем везде 0.5f, чтобы вещественные числа правильно округлялись при преобразовании к целому типу
				int(A[i].x + 0.5f), 
				int(A[i].y + 0.5f), 
				int(A[i2].x + 0.5f), 
				int(A[i2].y + 0.5f), COLOR(200, 30, 45));
		}
		
		// Рисуем описанную окружность
		frame.Circle((int)C.x, (int)C.y, int(a*sqrt(2) + 0.5f), COLOR(100, 100, 250));

		// Рисуем пиксель, на который кликнул пользователь
		if (global_clicked_pixel.X >= 0 && global_clicked_pixel.X < W &&
			global_clicked_pixel.Y >= 0 && global_clicked_pixel.Y < H)
			frame.SetPixel(global_clicked_pixel.X, global_clicked_pixel.Y, { 34, 175, 60 }); // Пиксель зелёного цвета
	}
};

#endif // PAINTER_H