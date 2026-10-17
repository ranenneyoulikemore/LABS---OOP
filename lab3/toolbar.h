#ifndef TOOLBAR_H
#define TOOLBAR_H

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#include <windows.h>
#include <commctrl.h>
#include "Shape.h"

#ifndef BTNS_CHECKGROUP
#define BTNS_CHECKGROUP 0x0006
#endif

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

#define IDM_POINT    1001
#define IDM_LINE     1002
#define IDM_RECT     1003
#define IDM_ELLIPSE  1004
#define IDM_EXIT     1005
#define IDM_ABOUT    1006
#define ID_TOOLBAR   2001

class Toolbar {
    HWND       hwnd;
    HIMAGELIST hImages;

    static const int ICON = 24;
    static const int COUNT = 4;

    void DrawIcons() {
        HDC scr = GetDC(NULL);
        HDC mem = CreateCompatibleDC(scr);
        HBITMAP bmp = CreateCompatibleBitmap(scr, ICON * COUNT, ICON);
        HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);

        // фон = маска (маджента)
        RECT all = { 0, 0, ICON * COUNT, ICON };
        HBRUSH bg = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(mem, &all, bg);
        DeleteObject(bg);

        HPEN   pen1 = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN   pen2 = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HBRUSH yellow = CreateSolidBrush(RGB(255, 255, 0));
        HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
        HPEN   oldPen = (HPEN)SelectObject(mem, pen1);
        HBRUSH oldBrush = (HBRUSH)SelectObject(mem, GetStockObject(NULL_BRUSH));

        int ox;

        ox = 0;
        MoveToEx(mem, ox + 12, 4, NULL);  LineTo(mem, ox + 12, 21);
        MoveToEx(mem, ox + 4, 12, NULL);  LineTo(mem, ox + 21, 12);
        SelectObject(mem, black);
        Ellipse(mem, ox + 9, 9, ox + 16, 16);

        ox = ICON;
        SelectObject(mem, pen2);
        MoveToEx(mem, ox + 4, 20, NULL);  LineTo(mem, ox + 20, 4);

        ox = ICON * 2;
        SelectObject(mem, pen1);
        SelectObject(mem, yellow);
        Rectangle(mem, ox + 3, 6, ox + 21, 19);

        ox = ICON * 3;
        SelectObject(mem, pen2);
        SelectObject(mem, GetStockObject(NULL_BRUSH));
        Ellipse(mem, ox + 3, 5, ox + 21, 19);

        SelectObject(mem, oldPen);
        SelectObject(mem, oldBrush);
        DeleteObject(pen1); DeleteObject(pen2);
        DeleteObject(yellow); DeleteObject(black);
        SelectObject(mem, oldBmp);
        DeleteDC(mem);
        ReleaseDC(NULL, scr);

        hImages = ImageList_Create(ICON, ICON, ILC_COLOR24 | ILC_MASK, COUNT, 0);
        ImageList_AddMasked(hImages, bmp, RGB(255, 0, 255));
        DeleteObject(bmp);
    }

public:
    Toolbar() : hwnd(NULL), hImages(NULL) {}
    ~Toolbar() { if (hImages) ImageList_Destroy(hImages); }

    bool Create(HWND parent, HINSTANCE hInst) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_BAR_CLASSES };
        InitCommonControlsEx(&icc);

        hwnd = CreateWindowExW(0, TOOLBARCLASSNAMEW, NULL,
            WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_TOP,
            0, 0, 0, 0, parent, (HMENU)(UINT_PTR)ID_TOOLBAR, hInst, NULL);
        if (!hwnd) return false;

        SendMessageW(hwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

        DrawIcons();
        SendMessageW(hwnd, TB_SETIMAGELIST, 0, (LPARAM)hImages);

        TBBUTTON btn[COUNT] = {};
        const int ids[COUNT] = { IDM_POINT, IDM_LINE, IDM_RECT, IDM_ELLIPSE };
        for (int i = 0; i < COUNT; i++) {
            btn[i].iBitmap = i;
            btn[i].idCommand = ids[i];
            btn[i].fsState = TBSTATE_ENABLED;
            btn[i].fsStyle = BTNS_CHECKGROUP;     
        }
        btn[0].fsState |= TBSTATE_CHECKED;        
        SendMessageW(hwnd, TB_ADDBUTTONS, COUNT, (LPARAM)btn);
        return true;
    }

    void AutoSize() { SendMessageW(hwnd, TB_AUTOSIZE, 0, 0); }

    int Height() const {
        RECT r; GetWindowRect(hwnd, &r);
        return r.bottom - r.top;
    }

    void Check(int id) {
        SendMessageW(hwnd, TB_CHECKBUTTON, id, MAKELONG(TRUE, 0));
    }

    static const wchar_t* TipText(int id) {
        switch (id) {
        case IDM_POINT:   return L"Крапка";
        case IDM_LINE:    return L"Лінія";
        case IDM_RECT:    return L"Прямокутник";
        case IDM_ELLIPSE: return L"Еліпс";
        }
        return L"";
    }
};

#endif