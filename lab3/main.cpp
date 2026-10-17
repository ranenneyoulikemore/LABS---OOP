#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include "Shapes.h"
#include "Toolbar.h"

class MainWindow {
    static const int N = 112;

    HWND    hWnd;
    HINSTANCE hInst;
    HMENU   hObjectsMenu;
    Toolbar toolbar;

    Shape** pcshape;
    int     shapeCount;
    int     currentObjectType;
    bool    isDrawing;
    POINT   ptStart, ptCurrent;

    void DrawRubberTrace(POINT p1, POINT p2) {
        HDC hdc = GetDC(hWnd);
        SetROP2(hdc, R2_NOTXORPEN);

        HPEN   hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN   hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        if (currentObjectType == IDM_RECT) {
            Rectangle(hdc, std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                           std::max(p1.x, p2.x), std::max(p1.y, p2.y));
        }
        else if (currentObjectType == IDM_ELLIPSE) {
            long dx = std::abs(p2.x - p1.x);
            long dy = std::abs(p2.y - p1.y);
            Ellipse(hdc, p1.x - dx, p1.y - dy, p1.x + dx, p1.y + dy);
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
        hObjectsMenu = CreatePopupMenu();
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

    void AddShape(Shape* s, long x1, long y1, long x2, long y2) {
        if (shapeCount < N) {
            s->Set(x1, y1, x2, y2);
            pcshape[shapeCount++] = s;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else {
            delete s;
        }
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
            currentObjectType = id;
            toolbar.Check(id);
            break;
        case IDM_ABOUT:
            MessageBoxW(hWnd,
                L"Лабораторна робота №3\nВаріант №12\nВиконала: студентка Дрига Олександра ІМ-54",
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
            CheckMenuRadioItem(hMenu, IDM_POINT, IDM_ELLIPSE,
                               currentObjectType, MF_BYCOMMAND);
        }
        return 0;
    }

    LRESULT OnLButtonDown(LPARAM lParam) {
        ptStart.x = GET_X_LPARAM(lParam);
        ptStart.y = GET_Y_LPARAM(lParam);
        ptCurrent = ptStart;

        if (currentObjectType == IDM_POINT) {
            AddShape(new PointShape(), ptStart.x, ptStart.y, ptStart.x, ptStart.y);
            isDrawing = false;
        }
        else {
            isDrawing = true;
            SetCapture(hWnd);
            DrawRubberTrace(ptStart, ptCurrent);
        }
        return 0;
    }

    LRESULT OnMouseMove(LPARAM lParam) {
        if (isDrawing) {
            DrawRubberTrace(ptStart, ptCurrent);
            ptCurrent.x = GET_X_LPARAM(lParam);
            ptCurrent.y = GET_Y_LPARAM(lParam);
            DrawRubberTrace(ptStart, ptCurrent);
        }
        return 0;
    }

    LRESULT OnLButtonUp(LPARAM lParam) {
        if (isDrawing) {
            DrawRubberTrace(ptStart, ptCurrent);
            isDrawing = false;
            ReleaseCapture();

            ptCurrent.x = GET_X_LPARAM(lParam);
            ptCurrent.y = GET_Y_LPARAM(lParam);

            Shape* s = nullptr;
            switch (currentObjectType) {
            case IDM_LINE:    s = new LineShape();      break;
            case IDM_RECT:    s = new RectangleShape(); break;
            case IDM_ELLIPSE: s = new EllipseShape();   break;
            }
            if (s) AddShape(s, ptStart.x, ptStart.y, ptCurrent.x, ptCurrent.y);
        }
        return 0;
    }

    LRESULT OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        for (int i = 0; i < shapeCount; i++)
            if (pcshape[i]) pcshape[i]->Show(hdc);
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
    MainWindow(HINSTANCE h)
        : hWnd(NULL), hInst(h), hObjectsMenu(NULL), shapeCount(0),
          currentObjectType(IDM_POINT), isDrawing(false) {
        pcshape = new Shape * [N];
        for (int i = 0; i < N; i++) pcshape[i] = nullptr;
        ptStart.x = ptStart.y = ptCurrent.x = ptCurrent.y = 0;
    }

    ~MainWindow() {
        for (int i = 0; i < shapeCount; i++) delete pcshape[i];
        delete[] pcshape;
    }

    bool Create(int nCmdShow) {
        const wchar_t cls[] = L"Lab3WindowClass";

        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = cls;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, cls, L"Lab3 - Графічний редактор",
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