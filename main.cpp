#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include "MyEditor.h"
#include "Toolbar.h"

static MyEditor editor;

class MainWindow {
    HWND      hWnd;
    HINSTANCE hInst;
    HMENU     hObjectsMenu;
    Toolbar   toolbar;

    HMENU CreateAppMenu() {
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        hObjectsMenu = CreatePopupMenu();
        HMENU hHelpMenu = CreatePopupMenu();

        AppendMenuW(hFileMenu, MF_STRING, IDM_EXIT, L"Вихід");

        AppendMenuW(hObjectsMenu, MF_STRING, IDM_POINT, L"Крапка");
        AppendMenuW(hObjectsMenu, MF_STRING, IDM_LINE, L"Лінія");
        AppendMenuW(hObjectsMenu, MF_STRING, IDM_RECT, L"Прямокутник");
        AppendMenuW(hObjectsMenu, MF_STRING, IDM_ELLIPSE, L"Еліпс");
        AppendMenuW(hObjectsMenu, MF_STRING, IDM_LINECIRCLE, L"Лінія з кружечками");
        AppendMenuW(hObjectsMenu, MF_STRING, IDM_CUBE, L"Каркас куба");

        AppendMenuW(hHelpMenu, MF_STRING, IDM_ABOUT, L"Про програму");

        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"Файл");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hObjectsMenu, L"Об'єкти");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"Довідка");
        return hMenu;
    }

    LRESULT OnCreate() {
        SetMenu(hWnd, CreateAppMenu());
        toolbar.Create(hWnd, hInst);
        return 0;
    }

    LRESULT OnSize() {
        toolbar.AutoSize();
        return 0;
    }

    LRESULT OnCommand(WPARAM wParam) {
        int id = LOWORD(wParam);
        switch (id) {
        case IDM_POINT:
        case IDM_LINE:
        case IDM_RECT:
        case IDM_ELLIPSE:
        case IDM_LINECIRCLE:
        case IDM_CUBE:
            editor.SetType(id);
            toolbar.Check(id);
            break;
        case IDM_ABOUT:
            MessageBoxW(hWnd,
                L"Лабораторна робота №4\nВаріант №11\nВиконала: студентка Дрига Олександра ІМ-54",
                L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        return 0;
    }

    LRESULT OnNotify(LPARAM lParam) {
        LPNMHDR hdr = (LPNMHDR)lParam;
        if (hdr->code == TTN_GETDISPINFOW) {
            LPNMTTDISPINFOW tip = (LPNMTTDISPINFOW)lParam;
            tip->lpszText = (LPWSTR)Toolbar::TipText((int)hdr->idFrom);
        }
        return 0;
    }

    LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam) {
        HMENU hMenu = (HMENU)wParam;
        if (HIWORD(lParam) == 0 && hMenu == hObjectsMenu) {
            CheckMenuRadioItem(hMenu, IDM_POINT, IDM_CUBE,
                               editor.GetType(), MF_BYCOMMAND);
        }
        return 0;
    }

    LRESULT OnLButtonDown(LPARAM lParam) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        editor.OnLButtonDown(hWnd, pt);
        return 0;
    }

    LRESULT OnMouseMove(LPARAM lParam) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        editor.OnMouseMove(hWnd, pt);
        return 0;
    }

    LRESULT OnLButtonUp(LPARAM lParam) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        editor.OnLButtonUp(hWnd, pt);
        return 0;
    }

    LRESULT OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        editor.OnPaint(hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }

    LRESULT OnDestroy() {
        PostQuitMessage(0);
        return 0;
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE:         return OnCreate();
        case WM_SIZE:           return OnSize();
        case WM_COMMAND:        return OnCommand(wParam);
        case WM_NOTIFY:         return OnNotify(lParam);
        case WM_INITMENUPOPUP:  return OnInitMenuPopup(wParam, lParam);
        case WM_LBUTTONDOWN:    return OnLButtonDown(lParam);
        case WM_MOUSEMOVE:      return OnMouseMove(lParam);
        case WM_LBUTTONUP:      return OnLButtonUp(lParam);
        case WM_PAINT:          return OnPaint();
        case WM_DESTROY:        return OnDestroy();
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        MainWindow* self = (MainWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if (msg == WM_NCCREATE) {
            self = (MainWindow*)((LPCREATESTRUCTW)lParam)->lpCreateParams;
            self->hWnd = hWnd;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)self);
        }
        if (self) return self->HandleMessage(msg, wParam, lParam);
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

public:
    MainWindow(HINSTANCE h) : hWnd(NULL), hInst(h), hObjectsMenu(NULL) {}

    bool Create(int nCmdShow) {
        const wchar_t cls[] = L"Lab4WindowClass";

        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = cls;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, cls, L"Lab4 - Графічний редактор",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            NULL, NULL, hInst, this);
        if (!h) return false;

        ShowWindow(h, nCmdShow);
        UpdateWindow(h);
        return true;
    }

    int Run() {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return (int)msg.wParam;
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    MainWindow app(hInstance);
    if (!app.Create(nCmdShow)) return 0;
    return app.Run();
}