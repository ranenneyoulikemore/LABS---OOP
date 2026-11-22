#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>
#include "Protocol.h"

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(w) ((short)HIWORD(w))
#endif

static std::string Fmt(double v) {
    char b[400];
    std::sprintf(b, "%.3f", v);
    return b;
}

static std::string IntA(int v) {
    char b[32];
    std::sprintf(b, "%d", v);
    return b;
}

static std::wstring Widen(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

class Object2Window {
    static const int HEADER = 32;

    HWND      hWnd;
    HINSTANCE hInst;
    HFONT     hFont;
    std::vector<double> values;
    double    minV, maxV;
    int       cellW, cellH, scrollPos;

    std::string Cell(int i) const {
        return "[" + IntA(i + 1) + "] " + Fmt(values[i]);
    }

    int Cols(const RECT& rc) const {
        int c = (rc.right - rc.left) / cellW;
        return c < 1 ? 1 : c;
    }

    int Rows(int cols) const {
        return ((int)values.size() + cols - 1) / cols;
    }

    int VisibleRows(const RECT& rc) const {
        int v = (rc.bottom - HEADER) / cellH;
        return v < 1 ? 1 : v;
    }

    void UpdateMetrics() {
        HDC hdc = GetDC(hWnd);
        HGDIOBJ old = SelectObject(hdc, hFont);
        TEXTMETRICW tm;
        GetTextMetricsW(hdc, &tm);
        cellH = tm.tmHeight + 6;
        int maxW = 0;
        for (int i = 0; i < (int)values.size(); i++) {
            std::string s = Cell(i);
            SIZE sz;
            GetTextExtentPoint32A(hdc, s.c_str(), (int)s.size(), &sz);
            maxW = std::max(maxW, (int)sz.cx);
        }
        cellW = std::max(maxW, 80) + 24;
        SelectObject(hdc, old);
        ReleaseDC(hWnd, hdc);
    }

    void UpdateScroll() {
        RECT rc;
        GetClientRect(hWnd, &rc);
        int rows = Rows(Cols(rc));
        int vis = VisibleRows(rc);
        int maxPos = std::max(0, rows - vis);
        scrollPos = std::max(0, std::min(scrollPos, maxPos));

        SCROLLINFO si = { sizeof(si) };
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
        si.nMin = 0;
        si.nMax = std::max(rows - 1, 0);
        si.nPage = vis;
        si.nPos = scrollPos;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
    }

    void SetScroll(int pos) {
        scrollPos = pos;
        UpdateScroll();
        InvalidateRect(hWnd, NULL, TRUE);
    }

    void Generate(const GenerateParams& p) {
        minV = std::min(p.minValue, p.maxValue);
        maxV = std::max(p.minValue, p.maxValue);
        values.assign(p.n, minV);
        if (minV < maxV) {
            std::mt19937 gen((unsigned)GetTickCount());
            std::uniform_real_distribution<double> dist(minV, maxV);
            for (double& v : values) v = std::floor(dist(gen) * 1000.0 + 0.5) / 1000.0;
        }
    }

    bool CopyToClipboard() {
        std::string text;
        for (double v : values) {
            text += Fmt(v);
            text += "\r\n";
        }
        bool opened = false;
        for (int i = 0; i < 10 && !opened; i++) {
            opened = OpenClipboard(hWnd) != 0;
            if (!opened) Sleep(30);
        }
        if (!opened) return false;
        EmptyClipboard();
        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (h) {
            char* p = (char*)GlobalLock(h);
            std::memcpy(p, text.c_str(), text.size() + 1);
            GlobalUnlock(h);
            if (!SetClipboardData(CF_TEXT, h)) GlobalFree(h);
        }
        CloseClipboard();
        return h != NULL;
    }

    LRESULT OnCreate() {
        hFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
        UpdateMetrics();
        UpdateScroll();
        return 0;
    }

    LRESULT OnCopyData(WPARAM sender, LPARAM lParam) {
        PCOPYDATASTRUCT cds = (PCOPYDATASTRUCT)lParam;
        if (!cds || cds->dwData != COPYDATA_GENERATE ||
            cds->cbData != sizeof(GenerateParams)) return FALSE;

        GenerateParams p = *(const GenerateParams*)cds->lpData;
        if (p.n < 1) return FALSE;

        Generate(p);
        CopyToClipboard();
        UpdateMetrics();
        scrollPos = 0;
        UpdateScroll();

        std::wstring title = L"Object2 - вектор (n = " + IntW(p.n) + L")";
        SetWindowTextW(hWnd, title.c_str());
        InvalidateRect(hWnd, NULL, TRUE);

        PostMessageW((HWND)sender, MsgDataReady(), (WPARAM)values.size(), 0);
        return TRUE;
    }

    LRESULT OnVScroll(WPARAM wParam) {
        SCROLLINFO si = { sizeof(si) };
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);
        int pos = si.nPos;
        switch (LOWORD(wParam)) {
        case SB_LINEUP:   pos--; break;
        case SB_LINEDOWN: pos++; break;
        case SB_PAGEUP:   pos -= (int)si.nPage; break;
        case SB_PAGEDOWN: pos += (int)si.nPage; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: pos = si.nTrackPos; break;
        case SB_TOP:      pos = 0; break;
        case SB_BOTTOM:   pos = si.nMax; break;
        }
        SetScroll(pos);
        return 0;
    }

    LRESULT OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        HGDIOBJ old = SelectObject(hdc, hFont);
        SetBkMode(hdc, TRANSPARENT);

        if (values.empty()) {
            const wchar_t* msg = L"Очікування команди від програми Lab6...";
            TextOutW(hdc, 10, 8, msg, (int)wcslen(msg));
        }
        else {
            std::wstring hdr = L"Вектор: n = " + IntW(values.size()) +
                L",  діапазон [" + Widen(Fmt(minV)) + L"; " + Widen(Fmt(maxV)) + L"]";
            TextOutW(hdc, 10, 6, hdr.c_str(), (int)hdr.size());
            MoveToEx(hdc, 0, HEADER - 4, NULL);
            LineTo(hdc, rc.right, HEADER - 4);

            int cols = Cols(rc);
            int rows = Rows(cols);
            int last = std::min(rows, scrollPos + VisibleRows(rc) + 1);
            for (int r = scrollPos; r < last; r++) {
                int y = HEADER + (r - scrollPos) * cellH;
                for (int c = 0; c < cols; c++) {
                    int idx = r * cols + c;
                    if (idx >= (int)values.size()) break;
                    std::string s = Cell(idx);
                    TextOutA(hdc, 10 + c * cellW, y, s.c_str(), (int)s.size());
                }
            }
        }
        SelectObject(hdc, old);
        EndPaint(hWnd, &ps);
        return 0;
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE:     return OnCreate();
        case WM_COPYDATA:   return OnCopyData(wParam, lParam);
        case WM_SIZE:       UpdateScroll(); InvalidateRect(hWnd, NULL, TRUE); return 0;
        case WM_VSCROLL:    return OnVScroll(wParam);
        case WM_MOUSEWHEEL:
            SetScroll(scrollPos - GET_WHEEL_DELTA_WPARAM(wParam) / 120 * 3);
            return 0;
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
        Object2Window* self = (Object2Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if (msg == WM_NCCREATE) {
            self = (Object2Window*)((LPCREATESTRUCTW)lParam)->lpCreateParams;
            self->hWnd = hWnd;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)self);
        }
        if (self) return self->HandleMessage(msg, wParam, lParam);
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

public:
    Object2Window(HINSTANCE h)
        : hWnd(NULL), hInst(h), hFont(NULL), minV(0), maxV(0),
          cellW(160), cellH(22), scrollPos(0) {}

    bool Create(int nCmdShow) {
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = CLS_OBJECT2;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, CLS_OBJECT2, L"Object2 - генератор вектора",
            WS_OVERLAPPEDWINDOW | WS_VSCROLL,
            CW_USEDEFAULT, CW_USEDEFAULT, 560, 420,
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
    Object2Window app(hInstance);
    if (!app.Create(nCmdShow)) return 0;
    return app.Run();
}