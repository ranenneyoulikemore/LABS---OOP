#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "Protocol.h"

static std::string Num(double v) {
    char b[64];
    std::sprintf(b, "%.6g", v);
    if (std::strcmp(b, "-0") == 0) return "0";
    return b;
}

static double NiceStep(double range, int target) {
    double raw = range / target;
    double mag = std::pow(10.0, std::floor(std::log10(raw)));
    double f = raw / mag;
    double nf = f < 1.5 ? 1 : f < 3 ? 2 : f < 7 ? 5 : 10;
    return nf * mag;
}

class Object3Window {
    HWND      hWnd;
    HINSTANCE hInst;
    HFONT     hFont;
    std::vector<double> data;

    bool ReadClipboard() {
        bool opened = false;
        for (int i = 0; i < 10 && !opened; i++) {
            opened = OpenClipboard(hWnd) != 0;
            if (!opened) Sleep(30);
        }
        if (!opened) return false;

        std::vector<double> parsed;
        HANDLE h = GetClipboardData(CF_TEXT);
        if (h) {
            const char* p = (const char*)GlobalLock(h);
            if (p) {
                while (*p) {
                    char* end;
                    double v = std::strtod(p, &end);
                    if (end == p) { p++; continue; }
                    parsed.push_back(v);
                    p = end;
                }
                GlobalUnlock(h);
            }
        }
        CloseClipboard();
        if (parsed.empty()) return false;
        data = parsed;
        return true;
    }

    void DrawGraph(HDC hdc, const RECT& rc) {
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        HGDIOBJ oldFont = SelectObject(hdc, hFont);
        SetBkMode(hdc, TRANSPARENT);
        TEXTMETRICW tm;
        GetTextMetricsW(hdc, &tm);
        int th = tm.tmHeight;

        if (data.empty()) {
            const wchar_t* msg = L"Очікування даних з буфера обміну...";
            TextOutW(hdc, 10, 8, msg, (int)wcslen(msg));
            SelectObject(hdc, oldFont);
            return;
        }

        const int padL = 70, padR = 35, padT = 56, padB = 50;
        int pw = rc.right - padL - padR;
        int ph = rc.bottom - padT - padB;
        if (pw < 60 || ph < 60) { SelectObject(hdc, oldFont); return; }

        int n = (int)data.size();
        double xmin = 0, xmax = std::max(n, 2);
        double dmin = *std::min_element(data.begin(), data.end());
        double dmax = *std::max_element(data.begin(), data.end());
        double ymin = std::min(0.0, dmin), ymax = std::max(0.0, dmax);
        if (ymax - ymin < 1e-9) ymax = ymin + 1;
        double span = ymax - ymin;
        if (ymin < 0) ymin -= span * 0.06;
        if (ymax > 0) ymax += span * 0.06;

        auto X = [&](double x) { return padL + (int)std::floor((x - xmin) / (xmax - xmin) * pw + 0.5); };
        auto Y = [&](double y) { return padT + (int)std::floor((ymax - y) / (ymax - ymin) * ph + 0.5); };

        int x0 = X(0), y0 = Y(0);
        double xs = std::max(1.0, NiceStep(xmax - xmin, std::max(2, pw / 80)));
        double ys = NiceStep(ymax - ymin, std::max(2, ph / 50));

        HPEN penGrid = CreatePen(PS_SOLID, 1, RGB(225, 225, 225));
        HPEN penAxis = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HPEN penLine = CreatePen(PS_SOLID, 2, RGB(0, 90, 200));
        HPEN penThin = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, penGrid);

        for (int k = 1; k * xs <= xmax + 1e-9; k++) {
            int px = X(k * xs);
            MoveToEx(hdc, px, padT - 8, NULL);
            LineTo(hdc, px, padT + ph);
        }
        for (int k = (int)std::ceil(ymin / ys); k * ys <= ymax + 1e-9; k++) {
            int py = Y(k * ys);
            MoveToEx(hdc, padL, py, NULL);
            LineTo(hdc, padL + pw + 8, py);
        }

        SelectObject(hdc, penAxis);
        MoveToEx(hdc, padL, y0, NULL);       LineTo(hdc, padL + pw + 14, y0);
        MoveToEx(hdc, x0, padT + ph, NULL);  LineTo(hdc, x0, padT - 14);
        SelectObject(hdc, penThin);
        MoveToEx(hdc, padL + pw + 14, y0, NULL); LineTo(hdc, padL + pw + 6, y0 - 4);
        MoveToEx(hdc, padL + pw + 14, y0, NULL); LineTo(hdc, padL + pw + 6, y0 + 4);
        MoveToEx(hdc, x0, padT - 14, NULL);  LineTo(hdc, x0 - 4, padT - 6);
        MoveToEx(hdc, x0, padT - 14, NULL);  LineTo(hdc, x0 + 4, padT - 6);

        SetTextColor(hdc, RGB(0, 0, 0));
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        for (int k = 1; k * xs <= xmax + 1e-9; k++) {
            int px = X(k * xs);
            MoveToEx(hdc, px, y0 - 3, NULL);
            LineTo(hdc, px, y0 + 4);
            std::string s = Num(k * xs);
            TextOutA(hdc, px, y0 + 6, s.c_str(), (int)s.size());
        }
        SetTextAlign(hdc, TA_RIGHT | TA_TOP);
        for (int k = (int)std::ceil(ymin / ys); k * ys <= ymax + 1e-9; k++) {
            int py = Y(k * ys);
            MoveToEx(hdc, x0 - 3, py, NULL);
            LineTo(hdc, x0 + 4, py);
            std::string s = Num(k * ys);
            TextOutA(hdc, x0 - 8, py - th / 2, s.c_str(), (int)s.size());
        }

        SetTextAlign(hdc, TA_LEFT | TA_TOP);
        TextOutA(hdc, padL + pw + 12, y0 + 8, "x", 1);
        TextOutA(hdc, x0 + 10, padT - 30, "y", 1);
        const wchar_t* title = L"Графік y = f(x), x - номер елемента вектора";
        TextOutW(hdc, 10, 6, title, (int)wcslen(title));

        std::vector<POINT> pts(n);
        for (int i = 0; i < n; i++) {
            pts[i].x = X(i + 1);
            pts[i].y = Y(data[i]);
        }
        SelectObject(hdc, penLine);
        Polyline(hdc, pts.data(), n);

        if (n <= 80) {
            HBRUSH br = CreateSolidBrush(RGB(0, 90, 200));
            HGDIOBJ oldBr = SelectObject(hdc, br);
            for (int i = 0; i < n; i++)
                Ellipse(hdc, pts[i].x - 3, pts[i].y - 3, pts[i].x + 4, pts[i].y + 4);
            SelectObject(hdc, oldBr);
            DeleteObject(br);
        }

        SelectObject(hdc, oldPen);
        DeleteObject(penGrid); DeleteObject(penAxis);
        DeleteObject(penLine); DeleteObject(penThin);
        SelectObject(hdc, oldFont);
    }

    LRESULT OnShowGraph(WPARAM sender) {
        bool ok = ReadClipboard();
        InvalidateRect(hWnd, NULL, TRUE);
        std::wstring title = ok
            ? L"Object3 - графік (точок: " + IntW(data.size()) + L")"
            : L"Object3 - графік (помилка читання буфера обміну)";
        SetWindowTextW(hWnd, title.c_str());
        PostMessageW((HWND)sender, MsgGraphDone(), ok ? (WPARAM)data.size() : 0, ok ? 0 : 1);
        return 0;
    }

    LRESULT OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HGDIOBJ old = SelectObject(mem, bmp);
        DrawGraph(mem, rc);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hWnd, &ps);
        return 0;
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == MsgShowGraph()) return OnShowGraph(wParam);

        switch (msg) {
        case WM_CREATE:
            hFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            return 0;
        case WM_SIZE:       InvalidateRect(hWnd, NULL, FALSE); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT:      return OnPaint();
        case WM_CLOSE:      DestroyWindow(hWnd); return 0;
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Object3Window* self = (Object3Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if (msg == WM_NCCREATE) {
            self = (Object3Window*)((LPCREATESTRUCTW)lParam)->lpCreateParams;
            self->hWnd = hWnd;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)self);
        }
        if (self) return self->HandleMessage(msg, wParam, lParam);
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

public:
    Object3Window(HINSTANCE h) : hWnd(NULL), hInst(h), hFont(NULL) {}

    bool Create(int nCmdShow) {
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = CLS_OBJECT3;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, CLS_OBJECT3, L"Object3 - графік",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 420,
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
    Object3Window app(hInstance);
    if (!app.Create(nCmdShow)) return 0;
    return app.Run();
}