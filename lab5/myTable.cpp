#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif
#include "MyTable.h"
#include <commctrl.h>
#include <sstream>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

static const wchar_t kTableClass[] = L"Lab5TableWindow";

template <typename T>
static std::wstring ToWString(const T& val) {
    std::wstringstream wss;
    wss << val;
    return wss.str();
}

MyTable::MyTable()
    : hwnd(NULL), hList(NULL), hInst(NULL), listener(NULL), updating(false) {}

MyTable::~MyTable() {
    listener = NULL;
    Close();
}

bool MyTable::Open(HWND owner, HINSTANCE h) {
    if (hwnd) { SetForegroundWindow(hwnd); return true; }
    hInst = h;

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icc);

    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = kTableClass;
        if (!RegisterClassExW(&wc)) return false;
        registered = true;
    }

    RECT rc = { 100, 100, 0, 0 };
    if (owner) GetWindowRect(owner, &rc);

    HWND w = CreateWindowExW(WS_EX_TOOLWINDOW, kTableClass,
        L"Таблиця графічних об'єктів",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        rc.left + 60, rc.top + 90, 440, 300,
        owner, NULL, hInst, this);
    if (!w) return false;

    ShowWindow(w, SW_SHOW);
    UpdateWindow(w);
    return true;
}

void MyTable::Close() {
    if (hwnd) DestroyWindow(hwnd);
}

void MyTable::SetRows(const std::vector<Row>& newRows) {
    rows = newRows;
    if (hwnd) Fill();
}

void MyTable::Fill() {
    updating = true;
    ListView_DeleteAllItems(hList);
    for (int i = 0; i < (int)rows.size(); i++) {
        LVITEMW it = { 0 };
        it.mask = LVIF_TEXT;
        it.iItem = i;
        it.pszText = (LPWSTR)rows[i].name.c_str();
        ListView_InsertItem(hList, &it);

        const long v[4] = { rows[i].x1, rows[i].y1, rows[i].x2, rows[i].y2 };
        for (int c = 0; c < 4; c++) {
            std::wstring s = ToWString(v[c]);
            ListView_SetItemText(hList, i, c + 1, (LPWSTR)s.c_str());
        }
    }
    updating = false;
}

LRESULT MyTable::HandleMessage(HWND h, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            0, 0, 0, 0, h, (HMENU)1, hInst, NULL);
        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        const wchar_t* titles[5] = { L"Назва", L"x1", L"y1", L"x2", L"y2" };
        const int widths[5] = { 150, 60, 60, 60, 60 };
        for (int i = 0; i < 5; i++) {
            LVCOLUMNW col = { 0 };
            col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
            col.pszText = (LPWSTR)titles[i];
            col.cx = widths[i];
            col.iSubItem = i;
            ListView_InsertColumn(hList, i, &col);
        }
        Fill();
        return 0;
    }
    case WM_SIZE:
        if (hList) MoveWindow(hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr->hwndFrom != hList || !listener) break;

        if (hdr->code == LVN_ITEMCHANGED && !updating) {
            NMLISTVIEW* nm = (NMLISTVIEW*)lParam;
            if (nm->uChanged & LVIF_STATE) {
                if (nm->uNewState & LVIS_SELECTED)
                    listener->OnRowSelected(nm->iItem);
                else if ((nm->uOldState & LVIS_SELECTED) && ListView_GetSelectedCount(hList) == 0)
                    listener->OnRowSelected(-1);
            }
        }
        else if (hdr->code == LVN_KEYDOWN) {
            NMLVKEYDOWN* kd = (NMLVKEYDOWN*)lParam;
            if (kd->wVKey == VK_DELETE) {
                int i = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
                if (i >= 0) listener->OnRowDeleteRequested(i);
            }
        }
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(h);
        return 0;

    case WM_NCDESTROY:
        hwnd = NULL;
        hList = NULL;
        SetWindowLongPtrW(h, GWLP_USERDATA, 0);
        if (listener) listener->OnTableClosed();
        break;
    }
    return DefWindowProcW(h, msg, wParam, lParam);
}

LRESULT CALLBACK MyTable::WndProc(HWND h, UINT msg, WPARAM wParam, LPARAM lParam) {
    MyTable* self = (MyTable*)GetWindowLongPtrW(h, GWLP_USERDATA);
    if (msg == WM_NCCREATE) {
        self = (MyTable*)((LPCREATESTRUCTW)lParam)->lpCreateParams;
        self->hwnd = h;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)self);
    }
    if (self) return self->HandleMessage(h, msg, wParam, lParam);
    return DefWindowProcW(h, msg, wParam, lParam);
}