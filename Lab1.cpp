#include <windows.h>
#include "utils.h"
#include "module1.h"
#include "module2.h"
#include "resource.h"

HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
    wcex.lpszClassName = L"Lab1Class";
    RegisterClassExW(&wcex);

    HWND hWnd = CreateWindowW(L"Lab1Class", L"Лабораторна робота 1 - Варіант 11", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 500, 300, nullptr, nullptr, hInstance, nullptr);

    HMENU hMenu = GetMenu(hWnd);
    if (hMenu) {
        ModifyMenuW(hMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 0), L"Меню");
        HMENU hSub = GetSubMenu(hMenu, 0);
        ModifyMenuW(hSub, IDM_WORK1, MF_BYCOMMAND | MF_STRING, IDM_WORK1, L"Робота1");
        ModifyMenuW(hSub, IDM_WORK2, MF_BYCOMMAND | MF_STRING, IDM_WORK2, L"Робота2");
        ModifyMenuW(hSub, IDM_EXIT, MF_BYCOMMAND | MF_STRING, IDM_EXIT, L"Вихід");
        DrawMenuBar(hWnd);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_WORK1:
            RunWork1(hInst, hWnd);
            break;
        case IDM_WORK2:
            RunWork2(hInst, hWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        TextOutW(hdc, 20, 50, g_MainDisplayText.c_str(), g_MainDisplayText.length());
        EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}