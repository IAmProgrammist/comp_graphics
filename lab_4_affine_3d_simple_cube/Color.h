#pragma once

#include <math.h>

// Cтруктура для задания цвета
struct COLOR
{
	unsigned char RED;		// Компонента красного цвета
	unsigned char GREEN;	// Компонента зелёного цвета
	unsigned char BLUE;		// Компонента синего цвета
	unsigned char ALPHA;	// Прозрачность (альфа канал)

	COLOR(int red, int green, int blue, int alpha = 255)
		: RED(red), GREEN(green), BLUE(blue), ALPHA(alpha) { }

} ;


// Пиксель в буфере кадра
struct PIXEL
{
	unsigned char RED;		// Компонента красного цвета
	unsigned char GREEN;	// Компонента зелёного цвета
	unsigned char BLUE;		// Компонента синего цвета
	float Z;				// Глубина пикселя
	PIXEL() : RED(0), GREEN(0), BLUE(0), Z(INFINITY) { }
} ;