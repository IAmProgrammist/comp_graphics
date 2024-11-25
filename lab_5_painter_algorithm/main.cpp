// main.cpp : Определяет точку входа для приложения.

#include <Windows.h>
#include <commctrl.h>
#include "stdio.h"
#include "Frame.h"
#include "Painter.h"

// Windows-приложение для создания простейших трёхмерных объектов

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hWndStatusBar; // Дескриптор компонента StatusBar

bool lmbPressed = false;
bool lmbFirst = true;
int prevMouseX = 0;
int prevMouseY = 0;

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	char szWinName[] = "Graphics Window Class"; // Имя класса окна

	HWND hWnd; // Дескриптор главного окна
	MSG msg;
	WNDCLASSA wcl; // Определитель класса окна
	wcl.hInstance = hThisInstance; // Дескриптор приложения
	wcl.lpszClassName = szWinName;// Имя класса окна
	wcl.lpfnWndProc = WindowProc; // Функция обработки сообщений
	wcl.style = 0; // Стиль по умолчанию
	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);// Иконка
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW); // Курсор
	wcl.lpszMenuName = NULL; // Без меню
	wcl.cbClsExtra = 0; // Без дополнительной информации
	wcl.cbWndExtra = 0;

	wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //Белый фон

	if (!RegisterClassA(&wcl)) // Регистрируем класс окна
		return 0;

	hWnd = CreateWindowA(szWinName, // Создать окно
		"Лабораторная работа №5. Алгоритмы удаления невидимых поверхностей",
		WS_OVERLAPPEDWINDOW, // Стиль окна
		CW_USEDEFAULT, // x-координата
		CW_USEDEFAULT, // y-координата
		CW_USEDEFAULT, // Ширина
		CW_USEDEFAULT, // Высота
		HWND_DESKTOP, // Без родительского окна
		NULL, // Без меню
		hThisInstance, // Дескриптор приложения
		NULL); // Без дополнительных аргументов

	// Создаём компонент типа StatusBar
	hWndStatusBar = CreateWindowExA(
		0, STATUSCLASSNAMEA, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0,
		hWnd, (HMENU)10001,
		hThisInstance, NULL
	);

	// Настройка частей StatusBar'а
	int statwidths[] = { 150, 300, -1 };
	SendMessageA(hWndStatusBar, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);

	ShowWindow(hWnd, nWinMode); // Показать окно
	UpdateWindow(hWnd); // Перерисовать окно

	while (GetMessage(&msg, NULL, 0, 0)) // Запустить цикл обработки сообщений
	{
		TranslateMessage(&msg); // Разрешить использование клавиатуры
		DispatchMessage(&msg); // Вернуть управление операционной системе Windows
	}

	return msg.wParam;

}


// Следующая функция вызывается операционной системой Windows и получает в качестве
// параметров сообщения из очереди сообщений данного приложения
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int pixelSize = 1; // Размер "большого" пикселя

	static DWORD start_time; // Начальный момент запуска программы
	float OFFSET_DELTA = 0.05;

	switch (message)
	{
		// Обработка сообщения на создание окна
	case WM_CREATE:
	{
		// Создаем таймер, посылающий сообщения
		// функции окна примерно 30 раз в секунду
		SetTimer(hWnd, 1, 1000 / 30, NULL);

		start_time = GetTickCount();
	}
	break;

	case WM_LBUTTONDOWN:
	{
		lmbPressed = true;
	}
	break;

	case WM_LBUTTONUP:
	{
		lmbPressed = false;
		lmbFirst = true;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (lmbPressed) {
			int xMouseMove = LOWORD(lParam), yMouseMove = HIWORD(lParam);

			if (lmbFirst) {
				prevMouseX = xMouseMove;
				prevMouseY = yMouseMove;
				lmbFirst = false;
			}
			else {
				y_rot += (xMouseMove - prevMouseX) / 100.0;
				x_rot += (yMouseMove - prevMouseY) / 100.0;
				prevMouseX = xMouseMove;
				prevMouseY = yMouseMove;
			}
		}
	}
	break;

	// Обработка сообщения на перерисовку окна
	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		HDC hdc = BeginPaint(hWnd, &ps);

		// Определяем ширину и высоту окна
		RECT rect = ps.rcPaint;
		GetClientRect(hWnd, &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		// Рисование в буфер кадра

		int ratio = pixelSize; // Размер "большого" пикселя

		int W = width / ratio;
		int H = (height - 22) / ratio; // Отнимем высоту StatusBar'а
		if (H < 0) H = 0;
		Frame frame(W, H, Perspective::FRUSTUM, 
			Matrix::RotationX(x_rot) * 
			Matrix::RotationY(y_rot) *
			Matrix::Translation(0, -.5, z_offset));

		Painter painter;

		// Вычислим время, которое нужно затратить для рисования одного кадра
		char repaint_time[500];
		DWORD t1 = GetTickCount();
		painter.Draw(frame);
		sprintf_s(repaint_time, "Время перерисовки: %d миллисекунд\n", GetTickCount() - t1);

		// Системная структура для хранения цвета пикселя
		// Буфер кадра, который будет передаваться операционной системе, должен состоять из массива этих структур
		// Она не совпадает с порядком следования цветов в формате RBG
		typedef struct tagRGBPIXEL
		{
			unsigned char BLUE;		// Компонента синего цвета
			unsigned char GREEN;	// Компонента зелёного цвета
			unsigned char RED;		// Компонента красного цвета
			unsigned char ALPHA;    // Прозрачность
		} RGBPIXEL;

		// Выделение памяти для второго буфера, который будет передаваться функции CreateBitmap для создания картинки
		size_t bitmap_size = width * height * sizeof(RGBPIXEL);

		RGBPIXEL* bitmap = (RGBPIXEL*)HeapAlloc(GetProcessHeap(), 0, bitmap_size);

		ZeroMemory(bitmap, bitmap_size);

		// Копирование массива пикселей в соответствии с системным форматом пикселя и масштабирование картинки
		// W и H - ширина и высота изображения в буфере кадра
		// ratio - коэффициент масштабирования пикселей
		int h = H * ratio - 1;
		for (int y = 0; y < H * ratio; y++)
			for (int x = 0; x < W * ratio; x++)
			{
				RGBPIXEL* pixel = bitmap + (size_t)(h - y) * width + x;
				COLOR color = frame.GetPixel(x / ratio, y / ratio);
				pixel->RED = color.RED;
				pixel->GREEN = color.GREEN;
				pixel->BLUE = color.BLUE;
				pixel->ALPHA = color.ALPHA;
			}



		// Получить дескриптор на новое растровое изображение
		HBITMAP hBitMap = CreateBitmap(width, height, 1, sizeof(RGBPIXEL) * 8, bitmap);

		// Освободить память, которую занимает буфер цвета
		HeapFree(GetProcessHeap(), 0, bitmap);

		// Создать в оперативной памяти контекст, совместимый с экранным контекстом, который мы используем, чтобы рисовать
		HDC srcHdc = CreateCompatibleDC(hdc);

		// Связать картинку с новым контекстом
		SelectObject(srcHdc, hBitMap);

		// Копировать содержимое из временного контекста srcHdc в основной контекст окна hdc
		BitBlt(
			hdc,        // Основной контекст
			0, 0,       // Координаты левого верхнего угла, от которого будет выполняться вставка
			width,      // Ширина вставляемого изображения
			height,     // Высота вставляемого изображения
			srcHdc,     // Дескриптор временного контекста
			0, 0,       // Координаты считываемого изображения
			SRCCOPY);   // Параметры операции - копирование 
		SetBkMode(hdc, TRANSPARENT); // Цвет фона, на котором будет написан текст 
		SetTextColor(hdc, RGB(255, 127, 40)); // Цвет текста
		DrawTextA(hdc, repaint_time, -1, &rect, 0);


		EndPaint(hWnd, &ps);

		// Удаление картинки из памяти
		DeleteObject(hBitMap);

		// Удаление временного контекста
		DeleteDC(srcHdc);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		auto wheeldata = GET_WHEEL_DELTA_WPARAM(wParam);
		if (wheeldata > 0) {
			z_offset += 0.01;
		}
		else if (wheeldata < 0) {
			z_offset -= 0.01;
		}
		// Перерисовать окно
		InvalidateRect(hWnd, NULL, false);
	}
	break;

	case WM_KEYDOWN:
		if (wParam == VK_F2 || wParam == VK_F3)
		{
			if (pixelSize > 1 && wParam == VK_F2) pixelSize--;
			if (pixelSize < 64 && wParam == VK_F3) pixelSize++;

			// Перерисовать окно
			InvalidateRect(hWnd, NULL, false);

			char str[256];
			sprintf_s(str, "Разрешение (F2/F3): %d", pixelSize);
			SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);
		}
		if (wParam == VK_F1)
		{
			MessageBoxA(hWnd, "Работу выполнил студент группы ПВ-223 Пахомов В. А.", "О программе", MB_ICONINFORMATION);
		}
		break;
	// Обработка сообщения на изменение размера окна
	case WM_SIZE:

		// Подгоняем размеры StatusBar под размер окна
		SendMessageA(hWndStatusBar, WM_SIZE, 0, 0);

		// Перерисовать окно
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_TIMER:

		// При срабатывании таймера пересчитаем время от запуска программы
		time = (GetTickCount() - start_time) / 1000.0;

		// Перерисовать окно
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_DESTROY: // Завершение программы
		PostQuitMessage(0);
		break;

	default:
		// Все сообщения, не обрабатываемые в данной функции, направляются на обработку по умолчанию
		return DefWindowProcA(hWnd, message, wParam, lParam);
	}

	return 0;
}

