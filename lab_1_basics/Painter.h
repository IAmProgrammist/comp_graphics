#ifndef PAINTER_H
#define PAINTER_H

#include "Frame.h"


// ���� �������� ������
float global_angle = 0;

// ���������� ���������� �������, ������� ������ ������������ 
struct
{
	int X, Y;
} global_clicked_pixel = {-1, -1};


class Painter
{
public:

	void Draw(Frame& frame)
	{
		// ��������� ��������
		for (int y = 0; y < frame.height; y++)
			for (int x = 0; x < frame.width; x++)
			{
				if ((x + y) % 2 == 0)
					frame.SetPixel(x, y, { 230, 255, 230 });	// ���������� ����
					//frame.SetPixel(x, y, { 217, 168, 14 });	
				else
					frame.SetPixel(x, y, { 200, 200, 200 }); // ׸���� ����
				//frame.SetPixel(x, y, { 255, 255, 255 }); // ����� ����
			}


		int W = frame.width, H = frame.height;
		// ������ ������� ������ ������ (7 / 8), ����� �� �� ������� ������ ������ 
		float a = 7.0f / 8 * ((W < H) ? W - 1 : H - 1) / sqrt(2);
		if (a < 1) return; // ���� ���� ����� ���������, �� ������ �� ������
		float angle = global_angle; // ���� ��������
		a = a / 2;
		
		// �������������� �������� ���������� ������ � ������ ��������
		struct
		{
			float x;
			float y;
		} C = {W / 2, H / 2}, A[4] = { { C.x + a, C.y + a}, {C.x + a, C.y - a}, {C.x - a, C.y - a}, {C.x - a, C.y + a} };
	

		// ������������ ��� ������� �������� ������ ����� C �� ���� angle
		for (int i = 0; i < 4; i++)
		{
			float xi = A[i].x, yi = A[i].y;
			A[i].x = (xi - C.x) * cos(angle) - (yi - C.y) * sin(angle) + C.x;
			A[i].y = (xi - C.x) * sin(angle) + (yi - C.y) * cos(angle) + C.y;
		}

		// ������ ������� ��������
		for (int i = 0; i < 4; i++)
		{
			int i2 = (i + 1) % 4;
			frame.DrawLine( // ��������� ����� 0.5f, ����� ������������ ����� ��������� ����������� ��� �������������� � ������ ����
				int(A[i].x + 0.5f), 
				int(A[i].y + 0.5f), 
				int(A[i2].x + 0.5f), 
				int(A[i2].y + 0.5f), COLOR(200, 30, 45));
		}
		
		// ������ ��������� ����������
		frame.Circle((int)C.x, (int)C.y, int(a*sqrt(2) + 0.5f), COLOR(100, 100, 250));

		// ������ �������, �� ������� ������� ������������
		if (global_clicked_pixel.X >= 0 && global_clicked_pixel.X < W &&
			global_clicked_pixel.Y >= 0 && global_clicked_pixel.Y < H)
			frame.SetPixel(global_clicked_pixel.X, global_clicked_pixel.Y, { 34, 175, 60 }); // ������� ������� �����
	}
};

#endif // PAINTER_H