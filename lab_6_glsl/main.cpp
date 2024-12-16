// main.cpp : Определяет точку входа для приложения.
#include <Windows.h>
#include <MMSystem.h>
#include <commctrl.h>
#include "Painter.h"

#include "gl/gl.h"
#include "AudioFile.h"
#include <fftw3.h>
#include "args.hxx"

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

HGLRC hglrc; // Контекст OpenGL 

void get_command_line_args(int* argc, char*** argv)
{
    // Get the command line arguments as wchar_t strings
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), argc);
    if (!wargv) { *argc = 0; *argv = NULL; return; }

    // Count the number of bytes necessary to store the UTF-8 versions of those strings
    int n = 0;
    for (int i = 0; i < *argc; i++)
        n += WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL) + 1;

    // Allocate the argv[] array + all the UTF-8 strings
    *argv = (char**)malloc((*argc + 1) * sizeof(char*) + n);
    if (!*argv) { *argc = 0; return; }

    // Convert all wargv[] --> argv[]
    char* arg = (char*)&((*argv)[*argc + 1]);
    for (int i = 0; i < *argc; i++)
    {
        (*argv)[i] = arg;
        arg += WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, arg, n, NULL, NULL) + 1;
    }
    (*argv)[*argc] = NULL;
}

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
    // Меняем цвет текста на красный
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stderr);  
    freopen_s(&stream, "CONOUT$", "w", stdout); 
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    int argc = 0;
    char **argv = NULL;
    get_command_line_args(&argc, &argv);

    args::ArgumentParser parser(
        "There Is Sound In Space! A sound visualization application made by Pakhomov Vladislav Andreevich", 
        "Have fun!");
    args::HelpFlag help(parser, "help", "Display help menu", { 'h', "help" });
    args::Positional<std::string> audioPathArg(parser, "audiopath", R"(
Required. Audio path to be played in .wav format with sample rate 44100 Hz. 
Potentially you can use bigger sample rate but it wasn't tested 
and could affect quality of frequencies recognition. 
Using audio with less frequency cuts down frequencies, so it's not recommended as well.
Use mono signal, since application can't work with stereo.
)");
    args::ValueFlag<int> maxTilesArg(parser, "maxtiles", R"(
Optional. Limiter of amount of tiles. Actual amount of tiles depends on samples amount that are
processed every step. Default = 64.
)", {'t', "tiles"});
    args::ValueFlag<double> gravityArg(parser, "gravity", R"(
Optional. This value affects how fast tiles are falling down. The bigger value the
bigger velocity of falling. Default = 0.001.
)", { 'g', "gravity" });
    args::ValueFlag<double> sensitivityArg(parser, "sensitivity", R"(
Optional. This value affects sensitivity of frequency amplitude. The bigger value the
bigger sensitivity. Default = 0.005.
)", { 's', "sensitivity" });
    args::ValueFlag<int> qualityArg(parser, "quality", R"(
Optional. Affects amount of samples being processed. Insert 1-15. Bigger value 
gives better amplitude recognition results but produces delays. Default = 14.
)", { 'q', "quality" });
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::ostringstream message;
        message << parser;
        MessageBoxA(NULL, message.str().c_str(), "Parse info", MB_ICONINFORMATION);
        return 0;
    }
    catch (args::ParseError e)
    {
        std::ostringstream message;
        message << e.what() << std::endl;
        message << parser;
        MessageBoxA(NULL, message.str().c_str(), "Parse info", MB_ICONINFORMATION);
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::ostringstream message;
        message << e.what() << std::endl;
        message << parser;
        MessageBoxA(NULL, message.str().c_str(), "Parse info", MB_ICONINFORMATION);
        return 1;
    }

    std::string audioPath;
    if (audioPathArg) {
        audioPath = args::get(audioPathArg);
    } else {
        MessageBoxA(NULL, "Audio path is required. Call program with argument -h for details", "Parse info", MB_ICONERROR);

        return 1;
    }

    if (maxTilesArg) {
        auto tmp = args::get(maxTilesArg);
        if (tmp > 0)
            MAX_TILES_AMOUNT = tmp;
    }

    if (gravityArg) {
        auto tmp = args::get(gravityArg);
        if (tmp > 0)
            GRAVITY = tmp;
    }

    if (sensitivityArg) {
        auto tmp = args::get(sensitivityArg);
        if (tmp > 0)
            SENSITIVITY = tmp;
    }

    if (qualityArg) {
        auto tmp = args::get(qualityArg);
        if (tmp > 0 && tmp < 16)
            SAMPLE_AMOUNT = 1 << tmp;
    }

    textureTile = BMP("textures\\tile.bmp");
    textureBackground = BMP("textures\\stars.bmp");

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
        "There Is Sound In Space!",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // Стиль окна
        CW_USEDEFAULT, // x-координата
        CW_USEDEFAULT, // y-координата
        CW_USEDEFAULT, // Ширина
        CW_USEDEFAULT, // Высота
        HWND_DESKTOP, // Без родительского окна
        NULL, // Без меню
        hThisInstance, // Дескриптор приложения
        NULL); // Без дополнительных аргументов

    ShowWindow(hWnd, nWinMode); // Показать окно

    setvbuf(stderr, NULL, _IONBF, 0); // Отключение буферизации потока ошибок stderr для того, чтобы лог-файл, в который выводится этот поток обновлялся сразу
    
    // Создание консоли
    // CreateLogConsole();
    
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

    audioFile.load(audioPath);
    PlaySoundA(audioPath.c_str(), hThisInstance, SND_FILENAME | SND_ASYNC);

    begin_time = GetTickCount();
    last_time = begin_time;

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
            SetTimer(hWnd, 1, 1000/170, NULL);

            start_time = GetTickCount();
            running_time = start_time;
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

            if (height < 0) height = 0;
                      
            // Вычислим время, которое нужно затратить для рисования одного кадра
            char repaint_time[500];
            DWORD t1 = GetTickCount();
            
            glViewport(0, 0, width, height); // Область вывода

            Draw(width, height);
            
            SwapBuffers(hdc); // Вывести содержимое буфера на экран
            
            EndPaint(hWnd, &ps);

        }
        break;

        case WM_MOUSEMOVE:
        {
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
                MessageBoxA(hWnd, "Made by student of group SC-223 Pakhomov Vladislav Andreevich", "About", MB_ICONINFORMATION);
            }

            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            break;

        // Обработка сообщения на изменение размера окна
        case WM_SIZE:

            // Перерисовать окно
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_TIMER:
        {
            // При срабатывании таймера пересчитаем время от запуска программы
            running_time = (GetTickCount() - start_time) / 1000.0f;

            float current_time = GetTickCount();
            float play_time_previous = last_time - begin_time;
            float play_time_current = current_time - begin_time;
            int slice_end = (play_time_current / 1000) * audioFile.getSampleRate();

            double exp_base_log = std::log(std::pow(MAX_FREQUENCY - MIN_FREQUENCY + 1, 1. / MAX_TILES_AMOUNT));

            if (slice_end > SAMPLE_AMOUNT) {
                // Create an array to hold the output (complex numbers)
                fftw_complex* output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * SAMPLE_AMOUNT);

                // Create a plan for the FFT
                fftw_plan plan = fftw_plan_dft_r2c_1d(SAMPLE_AMOUNT, audioFile.samples[0].data() - SAMPLE_AMOUNT + slice_end, output, FFTW_ESTIMATE);

                // Execute the FFT
                fftw_execute(plan);

                std::vector<std::pair<double, int>> prepared_freqs(MAX_TILES_AMOUNT, { 0, 0 });

                for (int i = 0; i < (SAMPLE_AMOUNT / 2) - 1; i++) {
                    double current_frequency = audioFile.getSampleRate() * i / SAMPLE_AMOUNT;

                    if ((current_frequency - MIN_FREQUENCY + 1) <= 0) continue;

                    double amplitude_index = std::log(current_frequency - MIN_FREQUENCY + 1) / exp_base_log;

                    if (amplitude_index < 0 || amplitude_index >= tiles_amplitudes.size()) break;

                    double amplitude = std::sqrt(std::pow(output[i][0], 2) + std::pow(output[i][1], 2));

                    prepared_freqs[amplitude_index].first += amplitude;
                    prepared_freqs[amplitude_index].second++;
                }

                int tmp_actual_tiles_amount = 0;

                for (int i = 0; i < MAX_TILES_AMOUNT; i++) {
                    if (prepared_freqs[i].second == 0) continue;

                    tiles_amplitudes[tmp_actual_tiles_amount] = std::max(
                        std::min(1., SENSITIVITY * prepared_freqs[i].first / prepared_freqs[i].second),
                        tiles_amplitudes[tmp_actual_tiles_amount] - (play_time_current - play_time_previous) * GRAVITY);

                    tmp_actual_tiles_amount++;
                }

                actual_tiles_amount = tmp_actual_tiles_amount;

                // Clean up
                fftw_destroy_plan(plan);
                fftw_free(output);
            }

            last_time = current_time;

            InvalidateRect(hWnd, NULL, false);
            break;
        }
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

