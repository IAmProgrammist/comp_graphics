// main.cpp : ���������� ����� ����� ��� ����������.

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


// ��������� Windows-���������� � �������������� ���������� OpenGL

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hWndStatusBar; // ���������� ���������� StatusBar

HGLRC hglrc; // �������� OpenGL 

// �������� ������� ��� ������ ��������� �� �������
void CreateLogConsole(void)
{
    // �������� ������� � ������������ � �� ����������� ����� ������
    AllocConsole();
    
    // ������ ������� �������� �� 1251 ��� ��������� ���������
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    // ������������� ����� ������� �� Consolas ��� ������ �����, �������������� ���������
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   /* CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);*/

    // ������ ���� ������ �� �������
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

    // �������������� ������ stderr � stdout � �������
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stderr);  // ��������������� ������ stderr
    freopen_s(&stream, "CONOUT$", "w", stdout);  // ��������������� ������ stdout

    //system("chcp 1251"); // ����������� ��������� �������
    /*
#ifdef _MSC_VER // ���� ��������� ������������� � MSVC 

    // ��������������� ������������ ������ ������ � ���� ������ Visual Studio
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
    char szWinName[] = "Graphics Window Class"; // ��� ������ ����

    HWND hWnd; // ���������� �������� ����

    WNDCLASSA wcl; // ������������ ������ ����
    wcl.hInstance = hThisInstance; // ���������� ����������
    wcl.lpszClassName = szWinName;// ��� ������ ����
    wcl.lpfnWndProc = WindowProc; // ������� ��������� ���������
    wcl.style = 0; // ����� �� ���������
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);// ������
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW); // ������
    wcl.lpszMenuName = NULL; // ��� ����
    wcl.cbClsExtra = 0; // ��� �������������� ����������
    wcl.cbWndExtra = 0;

    wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //����� ���

    if (!RegisterClassA(&wcl)) // ������������ ����� ����
        return 0;

    hWnd = CreateWindowA(szWinName, // ������� ����
        "������������ ������ �6. ��������� ���������� �� OpenGL",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // ����� ����
        CW_USEDEFAULT, // x-����������
        CW_USEDEFAULT, // y-����������
        CW_USEDEFAULT, // ������
        CW_USEDEFAULT, // ������
        HWND_DESKTOP, // ��� ������������� ����
        NULL, // ��� ����
        hThisInstance, // ���������� ����������
        NULL); // ��� �������������� ����������

    // ������ ��������� ���� StatusBar
    hWndStatusBar = CreateWindowExA(
        0, STATUSCLASSNAMEA, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU) 10001,
        hThisInstance, NULL
    );

    // ��������� ������ StatusBar'�
    int statwidths[] = { 150, 300, -1 };
    SendMessageA(hWndStatusBar, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);

    ShowWindow(hWnd, nWinMode); // �������� ����

    setvbuf(stderr, NULL, _IONBF, 0); // ���������� ����������� ������ ������ stderr ��� ����, ����� ���-����, � ������� ��������� ���� ����� ���������� �����
    
    // �������� �������
    //CreateLogConsole();
    
    // ��������� ��������� ���������� �����������
    HDC hdc = GetDC(hWnd);

    // ��������� ������� ��������
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);

    SetPixelFormat(hdc, pixelFormat, &pfd);

    // �������� ��������� OpenGL
    hglrc = wglCreateContext(hdc);

    wglMakeCurrent(hdc, hglrc);

    // ������������� ���������� GLEW
    glewInit();

    // ������������� OpenGL
    InitOpenGL();

    UpdateWindow(hWnd); // ������������ ����

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) // ��������� ���� ��������� ���������
    { 
        TranslateMessage(&msg); // ��������� ������������� ����������
        DispatchMessage(&msg); // ������� ���������� ������������ ������� Windows
    }

    return (int) msg.wParam;

}


// ��������� ������� ���������� ������������ �������� Windows � �������� � ��������
// ���������� ��������� �� ������� ��������� ������� ����������
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD start_time; // ��������� ������ ������� ���������

    switch (message)
    {
        // ��������� ��������� �� �������� ����
        case WM_CREATE:
        {
            // ������� ������, ���������� ���������
            // ������� ���� �������� 30 ��� � �������
            SetTimer(hWnd, 1, 1000/30, NULL);

            start_time = GetTickCount();
        }
        break;


        // ��������� ��������� �� ����������� ����
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            
            HDC hdc = BeginPaint(hWnd, &ps);

            // ���������� ������ � ������ ����
            RECT rect = ps.rcPaint;
            GetClientRect(hWnd, &rect);
         
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            // ������ ������ ������ �������
            RECT rect_status;
            GetWindowRect(hWndStatusBar, &rect_status);
            int statusBarHeight = rect_status.bottom - rect_status.top;

            height -= statusBarHeight; // ������� ������ StatusBar'�
            if (height < 0) height = 0;
                      
            // �������� �����, ������� ����� ��������� ��� ��������� ������ �����
            char repaint_time[500];
            DWORD t1 = GetTickCount();
            
            glViewport(0, statusBarHeight, width, height); // ������� ������

            Draw(width, height);
            
            SwapBuffers(hdc); // ������� ���������� ������ �� �����

            sprintf_s(repaint_time, "����� �����������: %d �����������\n�����: %0.3f ������\n", GetTickCount() - t1, running_time);
            
            // ����� ������
            //SetBkMode(hdc, TRANSPARENT); // ���� ����, �� ������� ����� ������� ����� 
            SetBkColor(hdc, RGB(0, 0, 0)); // ������ ���� ����
            SetTextColor(hdc, RGB(255, 127, 40)); // ���� ������
            DrawTextA(hdc, repaint_time, -1, &rect, 0); // ���������� �����
            
            EndPaint(hWnd, &ps);

        }
        break;

        case WM_MOUSEMOVE:
        {
            char str[256];

            // ������������� ����� � ������ ������ StatusBar'�
            // �������� ���������� ������� ����
            sprintf_s(str, "X = %d, Y = %d", LOWORD(lParam), HIWORD(lParam));
            SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);

            if (wParam == MK_LBUTTON)
            {
                // ��������, �� ������� ������������ ������ ���� ����� ����� ��������� WM_MOUSEMOVE
                int x = LOWORD(lParam), y = HIWORD(lParam);
                int dx = x - mousePosition.x;
                int dy = y - mousePosition.y;

                // ������� ������� �������� � ������������ � ���, ��� ������������ ���������� ������ ����
                changeRotateMatrix(dx, dy);

                // �������� ������� ������� ����
                mousePosition = { x, y };

                InvalidateRect(hWnd, NULL, false);
            }
        }
        break;

        case WM_LBUTTONDOWN:
            
            // ���������� ���������� ������� ���� ��� ������
            mousePosition = { LOWORD(lParam), HIWORD(lParam) };
            
            // ������������ ����
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_KEYDOWN:

			if (wParam == VK_F1)
            {
                MessageBoxA(hWnd, "������ �������� ������� ������ ��-223 ������� �.�.", "� ���������", MB_ICONINFORMATION);
            }

            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            break;

        // ��������� ��������� �� ��������� ������� ����
        case WM_SIZE:

            // ��������� ������� StatusBar'� ��� ������ ����
            SendMessageA(hWndStatusBar, WM_SIZE, 0, 0);

            // ������������ ����
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_TIMER:

            // ��� ������������ ������� ����������� ����� �� ������� ���������
            running_time = (GetTickCount() - start_time) / 1000.0f;
           // time += 0.1f;
            //time = 25.5;
            // ������������ ����
            InvalidateRect(hWnd, NULL, false);
            break;

        case WM_DESTROY: // ���������� ���������
            
            wglMakeCurrent(GetDC(hWnd), NULL);
            wglDeleteContext(hglrc);
            PostQuitMessage(0);
            break;

        default:
            // ��� ���������, �� �������������� � ������ �������, ������������ �� ��������� �� ���������
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}

