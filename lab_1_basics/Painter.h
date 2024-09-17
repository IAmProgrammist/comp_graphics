#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"
#include "Matrices.h"

// Установите 1 для отрисовки основного варианта, 0 - для отрисовки задания с защиты (сектор-круг)
#define MAIN_TASK 0


// Угол поворота фигуры
float global_angle = 0;

// Координаты последнего пикселя, который выбрал пользователь 
struct
{
	int X, Y;
} global_clicked_pixel = { -1, -1 };

typedef struct
{
	float x;
	float y;
} coordinate;

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
		float a = 7.0f / 8 * ((W < H) ? W - 1 : H - 1);
		if (a < 1) return; // Если окно очень маленькое, то ничего не рисуем
		float angle = -global_angle; // Угол поворота
		a = a / 2;
		coordinate C = { W / 2, H / 2 };


		// Код для отрисовки основного задания.
		if (MAIN_TASK) {
			// Рисуем описанную окружность
			frame.Circle((int)C.x, (int)C.y, (int)a, COLOR(0, 0, 0));
			// Рисуем вписанную окружность
			frame.Circle((int)C.x, (int)C.y, (int)(a * 0.5), COLOR(0, 0, 0));
			//Рисуем треугольник
			double t = (3 * a) / sqrt(3);
			coordinate triangleA = { C.x, C.y - a };
			coordinate triangleB = { C.x - t / 2, C.y + a / 2 };
			coordinate triangleC = { C.x + t / 2, C.y + a / 2 };
			frame.DrawLine(triangleA.x + 0.5, triangleA.y + 0.5, triangleB.x + 0.5, triangleB.y + 0.5, { 56, 93, 138 });
			frame.DrawLine(triangleC.x + 0.5, triangleC.y + 0.5, triangleB.x + 0.5, triangleB.y + 0.5, { 56, 93, 138 });
			frame.DrawLine(triangleA.x + 0.5, triangleA.y + 0.5, triangleC.x + 0.5, triangleC.y + 0.5, { 56, 93, 138 });

			Matrix S = { 1, 0, 0,
							0, 1, 0,
							0, 0, 1 };
			Matrix R = { cos(angle), -sin(angle),  0,
							sin(angle),  cos(angle),  0,
									0,           0,  1 };
			Matrix T = { 1, 0, W / 2.0,
							0, 1, H / 2.0,
							0, 0,       1 };
			Matrix SRT = (T.multiply(R)).multiply(S);
			double starOffset = a / 12;
			coordinate star[8] = {
				{ 0, a / 2 },
				{ starOffset, starOffset },
				{ a / 2, 0 },
				{ starOffset, -starOffset },
				{ 0, -a / 2 },
				{ -starOffset, -starOffset },
				{ -a / 2, 0 },
				{ -starOffset, starOffset } };

			for (int i = 0; i < 8; i++)
			{
				Vector pointVector = { star[i].x, star[i].y, 1 };
				pointVector = SRT.multiply(pointVector);
				star[i].x = pointVector.vector[0];
				star[i].y = pointVector.vector[1];
			}

			for (int i = 0; i < 8; i++)
			{
				int i2 = (i + 1) % 8;
				frame.DrawLine( // Добавляем везде 0.5f, чтобы вещественные числа правильно округлялись при преобразовании к целому типу
					int(star[i].x + 0.5f),
					int(star[i].y + 0.5f),
					int(star[i2].x + 0.5f),
					int(star[i2].y + 0.5f), COLOR(0, 176, 80));
			}
		}
		else {
			// Код для отрисовки сектор-круга: запрограммировать вручную.
			double startAngle = -2;
			double endAngle = startAngle + global_angle;
			if (endAngle > 3.14) endAngle = 3.14;
			frame.SectorCircle(startAngle, endAngle, (int)C.x, (int)C.y, (int)(a * 0.5), COLOR(0, 0, 0));
			frame.DrawLine(C.x, C.y, C.x + (int)(-a * 0.5 * sin(startAngle)), C.y + (int)(a * 0.5 * cos(startAngle)), COLOR(0, 0, 0));
			frame.DrawLine(C.x, C.y, C.x + (int)(-a * 0.5 * sin(endAngle)), C.y + (int)(a * 0.5 * cos(endAngle)), COLOR(0, 0, 0));
		}
		// Рисуем пиксель, на который кликнул пользователь
		if (global_clicked_pixel.X >= 0 && global_clicked_pixel.X < W &&
			global_clicked_pixel.Y >= 0 && global_clicked_pixel.Y < H)
			frame.SetPixel(global_clicked_pixel.X, global_clicked_pixel.Y, { 34, 175, 60 }); // Пиксель зелёного цвета
	}
};

#endif // PAINTER_H