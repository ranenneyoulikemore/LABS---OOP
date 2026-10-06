#include <windows.h>
#include <cwchar>
#include <cstdio>
#include "Shapes.h"

#define IDM_POINT    1001
#define IDM_LINE     1002
#define IDM_RECT     1003
#define IDM_ELLIPSE  1004
#define IDM_EXIT     1005
#define IDM_ABOUT    1006

#define N 111

Shape* pcshape[N];
int shapeCount = 0;

int currentObjectType = IDM_POINT;
bool isDrawing = false;
POINT ptStart, ptCurrent;

void UpdateWindowTitle(HWND hWnd) {
    const wchar_t* shapeName = L"Крапка";
    switch (currentObjectType) {
    case IDM_POINT:   shapeName = L"Крапка"; break;
    case IDM_LINE:    shapeName = L"Лінія"; break;
    case IDM_RECT:    shapeName = L"Прямокутник"; break;
    case IDM_ELLIPSE: shapeName = L"Еліпс"; break;
    }
    wchar_t title[256];
    swprintf_s(title, L"Lab2 - Графічний редактор [Поточний об'єкт: %s]", shapeName);
    SetWindowTextW(hWnd, title);
}

void DrawRubberTrace(HWND hWnd, POINT p1, POINT p2) {
    HDC hdc = GetDC(hWnd);
    SetROP2(hdc, R2_NOTXORPEN);

    HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    if (currentObjectType == IDM_RECT) {
        long dx = std::abs(p2.x - p1.x);
        long dy = std::abs(p2.y - p1.y);
        Rectangle(hdc, p1.x - dx, p1.y - dy, p1.x + dx, p1.y + dy);
    }
    else if (currentObjectType == IDM_ELLIPSE) {
        Ellipse(hdc, min(p1.x, p2.x), min(p1.y, p2.y), max(p1.x, p2.x), max(p1.y, p2.y));
    }
    else {
        MoveToEx(hdc, p1.x, p1.y, NULL);
        LineTo(hdc, p2.x, p2.y);
    }

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    ReleaseDC(hWnd, hdc);
}

HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hObjectsMenu = CreatePopupMenu();
    HMENU hHelpMenu = CreatePopupMenu();

    AppendMenuW(hFileMenu, MF_STRING, IDM_EXIT, L"Вихід");

    AppendMenuW(hObjectsMenu, MF_STRING, IDM_POINT, L"Крапка");
    AppendMenuW(hObjectsMenu, MF_STRING, IDM_LINE, L"Лінія");
    AppendMenuW(hObjectsMenu, MF_STRING, IDM_RECT, L"Прямокутник");
    AppendMenuW(hObjectsMenu, MF_STRING, IDM_ELLIPSE, L"Еліпс");

    AppendMenuW(hHelpMenu, MF_STRING, IDM_ABOUT, L"Про програму");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"Файл");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hObjectsMenu, L"Об'єкти");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"Довідка");

    return hMenu;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        SetMenu(hWnd, CreateAppMenu());
        UpdateWindowTitle(hWnd);
        break;

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_POINT:
        case IDM_LINE:
        case IDM_RECT:
        case IDM_ELLIPSE:
            currentObjectType = wmId;
            UpdateWindowTitle(hWnd);
            break;
        case IDM_ABOUT:
            MessageBoxW(hWnd, L"Лабораторна робота №2\nВаріант №11\nВиконала: студентка Дрига Олександра ІМ-54", L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    }

    case WM_LBUTTONDOWN:
        ptStart.x = LOWORD(lParam);
        ptStart.y = HIWORD(lParam);
        ptCurrent = ptStart;
        isDrawing = true;

        if (currentObjectType == IDM_POINT) {
            if (shapeCount < N) {
                pcshape[shapeCount] = new PointShape();
                pcshape[shapeCount]->Set(ptStart.x, ptStart.y, ptStart.x, ptStart.y);
                shapeCount++;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            isDrawing = false;
        }
        else {
            DrawRubberTrace(hWnd, ptStart, ptCurrent);
        }
        break;

    case WM_MOUSEMOVE:
        if (isDrawing) {
            DrawRubberTrace(hWnd, ptStart, ptCurrent);
            ptCurrent.x = LOWORD(lParam);
            ptCurrent.y = HIWORD(lParam);
            DrawRubberTrace(hWnd, ptStart, ptCurrent);
        }
        break;

    case WM_LBUTTONUP:
        if (isDrawing) {
            DrawRubberTrace(hWnd, ptStart, ptCurrent);
            isDrawing = false;

            ptCurrent.x = LOWORD(lParam);
            ptCurrent.y = HIWORD(lParam);

            if (shapeCount < N) {
                switch (currentObjectType) {
                case IDM_LINE:
                    pcshape[shapeCount] = new LineShape();
                    break;
                case IDM_RECT:
                    pcshape[shapeCount] = new RectangleShape();
                    break;
                case IDM_ELLIPSE:
                    pcshape[shapeCount] = new EllipseShape();
                    break;
                }
                pcshape[shapeCount]->Set(ptStart.x, ptStart.y, ptCurrent.x, ptCurrent.y);
                shapeCount++;
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        for (int i = 0; i < shapeCount; i++) {
            if (pcshape[i] != nullptr) {
                pcshape[i]->Show(hdc);
            }
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        for (int i = 0; i < shapeCount; i++) {
            delete pcshape[i];
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t szClassName[] = L"Lab2WindowClass";

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = szClassName;

    if (!RegisterClassExW(&wc)) return 0;

    HWND hWnd = CreateWindowExW(
        0, szClassName, L"Lab2", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}