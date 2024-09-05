// lab_1_basics.cpp : Определяет точку входа для приложения.


#include <Windows.h>
#include <commctrl.h>
#include "stdio.h"
#include "Frame.h"
#include "Painter.h"

// Windows-приложение для создания буфера кадра

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hWndStatusBar; // Дескриптор компонента StatusBar

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
        "Лабораторная работа №1. Буфер кадра. Алгоритмы Брезенхейма",
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
        hWnd, (HMENU) 10001,
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
    static int pixelSize = 8; // Размер "большого" пикселя

    switch (message)
    {
        // Обработка сообщения на создание окна
        case WM_CREATE:
        {
            // Создаем таймер, посылающий сообщения
            // функции окна примерно 30 раз в секунду
            SetTimer(hWnd, 1, 1000/30, NULL);
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

            static int W = 0;
            static int H = 0;
            static Frame* frame = new Frame(W, H); // Отнимем высоту StatusBar'а
            int newW = width / ratio;
            int newH = (height - 22) / ratio;
            if (newW != W || newH != H) {
                W = newW;
                H = newH;
                delete frame;
                frame = new Frame(W, H);
            }

            Painter painter;
            painter.Draw(*frame);

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
            RGBPIXEL* bitmap = (RGBPIXEL*) HeapAlloc(GetProcessHeap(), 0, width * height * sizeof(RGBPIXEL));

            // Копирование массива пикселей в соответствии с системным форматом пикселя и масштабирование картинки
            // W и H - ширина и высота изображения в буфере кадра
            // ratio - коэффициент масштабирования пикселей
            for (int y = 0; y < H * ratio; y++)
                for (int x = 0; x < W * ratio; x++)
                {
                    RGBPIXEL* pixel = bitmap + y * width + x;
                    COLOR color = frame->GetPixel(x / ratio, y / ratio);
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

            EndPaint(hWnd, &ps);

            // Удаление картинки из памяти
            DeleteObject(hBitMap);

            // Удаление временного контекста
            DeleteDC(srcHdc);
        }
        break;

        case WM_MOUSEMOVE:
        {
            char str[256];

            // Устанавливаем текст в разных частях StatusBar'а
            // Экранные координаты курсора мыши
            sprintf_s(str, "X = %d, Y = %d", LOWORD(lParam), HIWORD(lParam));
            SendMessageA(hWndStatusBar, SB_SETTEXTA, 2, (LPARAM)str);

            // Координаты пикселя в буфере кадра
            sprintf_s(str, "BX = %d, BY = %d", LOWORD(lParam) / pixelSize, HIWORD(lParam) / pixelSize);
            SendMessageA(hWndStatusBar, SB_SETTEXTA, 1, (LPARAM)str);

            sprintf_s(str, "Масштаб (F2/F3): %d", pixelSize);
            SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);
        }
        break;

        case WM_LBUTTONDOWN:
            // Запоминаем координаты пикселя, по которому щёлкнул пользователь
            global_clicked_pixel.X = LOWORD(lParam) / pixelSize;
            global_clicked_pixel.Y = HIWORD(lParam) / pixelSize;
            // Перерисовать окно
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_KEYDOWN:
            if (wParam == VK_F2 || wParam == VK_F3)
            {
                if (pixelSize > 1  && wParam == VK_F2) pixelSize--;
                if (pixelSize < 64 && wParam == VK_F3) pixelSize++;

                // Перерисовать окно
                InvalidateRect(hWnd, NULL, false);

                char str[256];
                sprintf_s(str, "Масштаб (F2/F3): %d", pixelSize);
                SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);
            }
            if (wParam == VK_F1)
            {
                MessageBoxA(hWnd, "Работу выполнил студент группы ПВ-221 Колесников А.И.", "О программе", MB_ICONINFORMATION);
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

            // При срабатывании таймера увеличим угол поворота
            global_angle += 0.05;
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

