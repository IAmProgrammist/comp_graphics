// main.cpp : Определяет точку входа для приложения.

#include <Windows.h>
#include <commctrl.h>
#include "Painter.h"

#include "gl/gl.h"

#include <iostream>

#if defined(_WIN64)
#pragma comment(lib, "win64_lib/opengl32.lib")
#pragma comment(lib, "win64_lib/glew32.lib")
#elif defined(_WIN32)
#pragma comment(lib, "win32_lib/opengl32.lib")
#pragma comment(lib, "win32_lib/glew32.lib")
#endif


// Каркасное Windows-приложение с использованием библиотеки OpenGL

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hWndStatusBar; // Дескриптор компонента StatusBar

HGLRC hglrc; // Контекст OpenGL 

// Создание консоли для вывода сообщений об ошибках
void CreateLogConsole(void)
{
    // Создадим консоль и перенаправим в неё стандартный поток ошибок
    AllocConsole();
    
    // Меняем кодовую страницу на 1251 для поддержки кириллицы
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    // Устанавливаем шрифт консоли на Consolas или другой шрифт, поддерживающий кириллицу
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   /* CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);*/

    // Меняем цвет текста на красный
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

    // Перенаправляем потоки stderr и stdout в консоль
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stderr);  // Перенаправление потока stderr
    freopen_s(&stream, "CONOUT$", "w", stdout);  // Перенаправление потока stdout

    //system("chcp 1251"); // Настраиваем кодировку консоли
    /*
#ifdef _MSC_VER // Если программа компилируется в MSVC 

    // Перенаправление стандартного потока вывода в окно вывода Visual Studio
    // https://gist.github.com/takashyx/937f3a794ad36cd98ec3
    class debugStreambuf : public std::streambuf  {
    public:
        virtual int_type overflow(int_type c = EOF) override {
            if (c != EOF) {
                TCHAR buf[] = { c, '\0' };
                OutputDebugString(buf);
            }
            return c;
        }
    };


    class Cout2VisualStudioDebugOutput {

        debugStreambuf dbgstream;
        std::streambuf* default_stream;

    public:
        Cout2VisualStudioDebugOutput() {
            default_stream = std::cerr.rdbuf(&dbgstream);
        }

        ~Cout2VisualStudioDebugOutput() {
            std::cerr.rdbuf(default_stream);
        }
    };

    Cout2VisualStudioDebugOutput* c2v = new Cout2VisualStudioDebugOutput();

    //std::cerr << "Standart Error write!" << std::endl;
   
#endif*/
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    char szWinName[] = "Graphics Window Class"; // Имя класса окна

    HWND hWnd; // Дескриптор главного окна

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
        "Лабораторная работа №6. Каркасное приложение на OpenGL",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // Стиль окна
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

    setvbuf(stderr, NULL, _IONBF, 0); // Отключение буферизации потока ошибок stderr для того, чтобы лог-файл, в который выводится этот поток обновлялся сразу
    
    // Создание консоли
    //CreateLogConsole();
    
    // Получение контекста устройства отображения
    HDC hdc = GetDC(hWnd);

    // Установка формата пикселей
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);

    SetPixelFormat(hdc, pixelFormat, &pfd);

    // Создание контекста OpenGL
    hglrc = wglCreateContext(hdc);

    wglMakeCurrent(hdc, hglrc);

    // Инициализация библиотеки GLEW
    glewInit();

    // Инициализация OpenGL
    InitOpenGL();

    UpdateWindow(hWnd); // Перерисовать окно

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) // Запустить цикл обработки сообщений
    { 
        TranslateMessage(&msg); // Разрешить использование клавиатуры
        DispatchMessage(&msg); // Вернуть управление операционной системе Windows
    }

    return (int) msg.wParam;

}


// Следующая функция вызывается операционной системой Windows и получает в качестве
// параметров сообщения из очереди сообщений данного приложения
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD start_time; // Начальный момент запуска программы

    switch (message)
    {
        // Обработка сообщения на создание окна
        case WM_CREATE:
        {
            // Создаем таймер, посылающий сообщения
            // функции окна примерно 30 раз в секунду
            SetTimer(hWnd, 1, 1000/30, NULL);

            start_time = GetTickCount();
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

            // Узнаем высоту строки статуса
            RECT rect_status;
            GetWindowRect(hWndStatusBar, &rect_status);
            int statusBarHeight = rect_status.bottom - rect_status.top;

            height -= statusBarHeight; // Отнимем высоту StatusBar'а
            if (height < 0) height = 0;
                      
            // Вычислим время, которое нужно затратить для рисования одного кадра
            char repaint_time[500];
            DWORD t1 = GetTickCount();
            
            glViewport(0, statusBarHeight, width, height); // Область вывода

            Draw(width, height);
            
            SwapBuffers(hdc); // Вывести содержимое буфера на экран

            sprintf_s(repaint_time, "Время перерисовки: %d миллисекунд\nВремя: %0.3f секунд\n", GetTickCount() - t1, running_time);
            
            // Вывод текста
            //SetBkMode(hdc, TRANSPARENT); // Цвет фона, на котором будет написан текст 
            SetBkColor(hdc, RGB(0, 0, 0)); // Черный цвет фона
            SetTextColor(hdc, RGB(255, 127, 40)); // Цвет текста
            DrawTextA(hdc, repaint_time, -1, &rect, 0); // Нарисовать текст
            
            EndPaint(hWnd, &ps);

        }
        break;

        case WM_MOUSEMOVE:
        {
            char str[256];

            // Устанавливаем текст в разных частях StatusBar'а
            // Экранные координаты курсора мыши
            sprintf_s(str, "X = %d, Y = %d", LOWORD(lParam), HIWORD(lParam));
            SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);

            if (wParam == MK_LBUTTON)
            {
                // Вычислим, на сколько переместился курсор мыши между двумя событиями WM_MOUSEMOVE
                int x = LOWORD(lParam), y = HIWORD(lParam);
                int dx = x - mousePosition.x;
                int dy = y - mousePosition.y;

                // Изменим матрицу поворота в соответствии с тем, как пользователь переместил курсор мыши
                changeRotateMatrix(dx, dy);

                // Сохраним текущую позицию мыши
                mousePosition = { x, y };

                InvalidateRect(hWnd, NULL, false);
            }
        }
        break;

        case WM_LBUTTONDOWN:
            
            // Запоминаем координаты курсора мыши при щелчке
            mousePosition = { LOWORD(lParam), HIWORD(lParam) };
            
            // Перерисовать окно
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_KEYDOWN:

			if (wParam == VK_F1)
            {
                MessageBoxA(hWnd, "Работу выполнил студент группы ПВ-223 Пахомов В.А.", "О программе", MB_ICONINFORMATION);
            }

            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            break;

        // Обработка сообщения на изменение размера окна
        case WM_SIZE:

            // Подгоняем размеры StatusBar'а под размер окна
            SendMessageA(hWndStatusBar, WM_SIZE, 0, 0);

            // Перерисовать окно
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_TIMER:

            // При срабатывании таймера пересчитаем время от запуска программы
            running_time = (GetTickCount() - start_time) / 1000.0f;
           // time += 0.1f;
            //time = 25.5;
            // Перерисовать окно
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_DESTROY: // Завершение программы
            
            wglMakeCurrent(GetDC(hWnd), NULL);
            wglDeleteContext(hglrc);
            PostQuitMessage(0);
            break;

        default:
            // Все сообщения, не обрабатываемые в данной функции, направляются на обработку по умолчанию
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}

