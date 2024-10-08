#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"
#include "Matrices.h"
#include "RadialInterpolator.h"
#include "BarycentricInterpolator.h"
#include "SectorInterpolator.h"
#include "ReuleauxTriangleInterpolator.h"

// Установите 1 для отрисовки основного варианта, 0 - для отрисовки задания с защиты (сектор-круг)
#define MAIN_TASK 1


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

	double sawtooth(double val, double height) {
		return abs(fmod(val, height) - height / 2);
	}

	void Draw(Frame& frame)
	{
		// Шахматная текстура
		for (int y = 0; y < frame.height; y++)
			for (int x = 0; x < frame.width; x++)
			{
				if ((x + y) % 2 == 0)
					frame.SetPixel(x, y, { 23, 25, 23 });	// Золотистый цвет
				//frame.SetPixel(x, y, { 217, 168, 14 });	
				else
					frame.SetPixel(x, y, { 20, 20, 20 }); // Чёрный цвет
				//frame.SetPixel(x, y, { 255, 255, 255 }); // Белый цвет
			}

		// Код для отрисовки основного задания.
		if (MAIN_TASK) {
			int W = frame.width - frame.width * 0.02, H = frame.height - frame.height * 0.02;
			double t1XOffset = frame.width * 0.01 + sawtooth(312 + global_angle * 65, W * 2), t1YOffset = frame.height * 0.01 + sawtooth(41 + global_angle * 131, H * 2);
			double t2XOffset = frame.width * 0.01 + sawtooth(19 + global_angle * 102, W * 2), t2YOffset = frame.height * 0.01 + sawtooth(901 + global_angle * 12, H * 2);
			double t3XOffset = frame.width * 0.01 + sawtooth(71 + global_angle * 25, W * 2), t3YOffset = frame.height * 0.01 + sawtooth(47 + global_angle * 32, H * 2);
			SectorInterpolator t1s(t1XOffset, t1YOffset);
			RadialInterpolator t2s(t2XOffset, t2YOffset, t2XOffset, t2YOffset, { {255, 180, 0}, {180, 0, 99}, {188, 188, 188} }, global_angle / 5);
			BarycentricInterpolator t3s(frame.width * 0.01, frame.height * 0.01, frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01,
				{ 91, 9, 39, 12 }, { 98, 192, 67 }, { 76, 100, 158 });
			ReuleauxTriangleInterpolator<SectorInterpolator> t1(t1XOffset, t1YOffset, 50 + sawtooth(global_angle * 10, 30), 12 + global_angle, 0.3, t1s);
			ReuleauxTriangleInterpolator<RadialInterpolator> t2(t2XOffset, t2YOffset, 70 + sawtooth(global_angle * 22, 11), 9 + 2 * global_angle, 0, t2s);
			ReuleauxTriangleInterpolator<BarycentricInterpolator> t3(t3XOffset, t3YOffset, 20 + sawtooth(global_angle * 10, 100), 12 + global_angle, 0.7, t3s);

			frame.Triangle(frame.width * 0.01, frame.height * 0.01, frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t1);
			frame.Triangle(frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t1);
			frame.Triangle(frame.width * 0.01, frame.height * 0.01, frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t2);
			frame.Triangle(frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t2);
			frame.Triangle(frame.width * 0.01, frame.height * 0.01, frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t3);
			frame.Triangle(frame.width * 0.01, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01 + H, frame.width * 0.01 + W, frame.height * 0.01, t3);
		}
		else {
		}
	}
};

#endif // PAINTER_H