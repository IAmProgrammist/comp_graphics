#ifndef FRAME_H
#define FRAME_H

#include <math.h>

// C�������� ��� ������� �����
typedef struct tagCOLOR
{
	unsigned char RED;		// ���������� �������� �����
	unsigned char GREEN;	// ���������� ������� �����
	unsigned char BLUE;		// ���������� ������ �����
	unsigned char ALPHA;	// ������������ (����� �����)

	tagCOLOR() : RED(0), GREEN(0), BLUE(0), ALPHA(255) { }
	tagCOLOR(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255) : RED(red), GREEN(green), BLUE(blue), ALPHA(alpha) { }

} COLOR;


template<typename TYPE> void swap(TYPE& a, TYPE& b)
{
	TYPE t = a;
	a = b;
	b = t;
}


// ����� �����
class Frame
{
	// ��������� �� ������ ��������
	// ����� ����� ����� ������������ ����� �������, ������� ������������� � ������ � ���� ������������ �����
	COLOR* pixels;

	// ��������� �� ������ �������� ������ �����
	COLOR** matrix;

public:

	// ������� ������ �����
	int width, height;

	Frame(int _width, int _height) : width(_width), height(_height)
	{
		int size = width * height;

		// �������� ������ ����� � ���� ����������� ������� ��������
		pixels = new COLOR[size];

		// ��������� �� ������ �������� ������� � ��������� ������
		matrix = new COLOR* [height];

		// ������������� ������� ����������
		for (int i = 0; i < height; i++)
		{
			matrix[i] = pixels + i * width;
		}
	}


	// ����� ���� color ������� � ������������ (x, y)
	void SetPixel(int x, int y, COLOR color)
	{
		matrix[y][x] = color;
	}

	// ���������� ���� ������� � ������������ (x, y)
	COLOR GetPixel(int x, int y) 
	{
		return matrix[y][x];
	}


	// ��������� ����������
	void Circle(int x0, int y0, int radius, COLOR color)
	{
		int x = 0, y = radius;
		while(x < y)
		{
			// ����������, ����� ����� (�������): (x, y) ��� (x, y - 1) ����� � ����� ����������
			int D1 = x * x + y * y - radius * radius;
			int D2 = x * x + (y - 1) * (y - 1) - radius * radius;
			
			// ���� ����� ����� (x, y - 1), �� ��������� � ���
			if (D1 > -D2)
				y--;

			// ������� � ��������� ����������� ��������� �� ��� ������� ���������� 
			SetPixel(x0 + x, y0 + y, color);
			SetPixel(x0 + x, y0 - y, color);
			SetPixel(x0 + y, y0 + x, color);
			SetPixel(x0 + y, y0 - x, color);
			SetPixel(x0 - x, y0 + y, color);
			SetPixel(x0 - x, y0 - y, color);
			SetPixel(x0 - y, y0 + x, color);
			SetPixel(x0 - y, y0 - x, color);
			x++;
		}
	}


	// ��������� �������
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
				// ����� ������� ����� (x1, y1) � (x2, y2)
				swap(x1, x2);
				swap(y1, y2);
				dx = -dx; dy = -dy;
			}
			
			int y, dx2 = dx / 2, p = 0;
			if (dy < 0) dx2 = -dx2;
			for (int x = x1; x <= x2; x++)
			{
				// y = (dy * (x - x1) + dx2) / dx + y1;
				y = (p + dx2) / dx + y1;
				p += dy;
				matrix[y][x] = color;
			}
		}
		else
		{ 
			if (y2 < y1)
			{
				// ����� ������� ����� (x1, y1) � (x2, y2)
				swap(x1, x2);
				swap(y1, y2);
				dx = -dx; dy = -dy;
			}

			int x, dy2 = dy / 2, p = 0;
			if (dx < 0) dy2 = -dy2;
			for (int y = y1; y <= y2; y++)
			{
				// x = (dx * (y - y1) + dy2) / dy + x1;
				x = (p + dy2) / dy + x1;
				p += dx;
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
