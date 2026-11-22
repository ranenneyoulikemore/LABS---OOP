#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Protocol.h"

#define IDM_RUN    101
#define IDM_EXIT   102
#define IDM_ABOUT  103

#define IDC_EDIT_N    201
#define IDC_EDIT_MIN  202
#define IDC_EDIT_MAX  203

static const wchar_t kDlgClass[] = L"Lab6InputDialog";

static std::wstring Widen(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

static std::wstring NumW(double v) {
    char b[64];
    std::sprintf(b, "%g", v);
    return Widen(b);
}

static std::wstring GetText(HWND h) {
    int len = GetWindowTextLengthW(h);
    std::wstring s(len + 1, L'\0');
    GetWindowTextW(h, &s[0], len + 1);
    s.resize(len);
    return s;
}

static bool ParseNumber(const std::wstring& in, double& v) {
    std::wstring s = in;
    for (wchar_t& c : s) if (c == L',') c = L'.';
    wchar_t* end = NULL;
    v = wcstod(s.c_str(), &end);
    if (end == s.c_str()) return false;
    while (*end == L' ') end++;
    return *end == 0;
}

struct DlgState {
    HWND            edit[3];
    GenerateParams* out;
    bool            ok;
    bool            done;
};

static HWND MakeControl(HWND parent, const wchar_t* cls, const wchar_t* text,
                        DWORD style, int x, int y, int w, int h, int id) {
    HWND c = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return c;
}

static LRESULT CALLBACK InputDlgProc(HWND h, UINT msg, WPARAM wParam, LPARAM lParam) {
    DlgState* st = (DlgState*)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (msg) {
    case WM_NCCREATE:
        st = (DlgState*)((LPCREATESTRUCTW)lParam)->lpCreateParams;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)st);
        break;

    case WM_CREATE:
        MakeControl(h, L"STATIC", L"n (кількість елементів):", 0, 15, 20, 170, 20, 0);
        MakeControl(h, L"STATIC", L"Min (мінімальне значення):", 0, 15, 55, 170, 20, 0);
        MakeControl(h, L"STATIC", L"Max (максимальне значення):", 0, 15, 90, 170, 20, 0);
        st->edit[0] = MakeControl(h, L"EDIT", L"30", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
                                  195, 17, 100, 22, IDC_EDIT_N);
        st->edit[1] = MakeControl(h, L"EDIT", L"-50", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
                                  195, 52, 100, 22, IDC_EDIT_MIN);
        st->edit[2] = MakeControl(h, L"EDIT", L"50", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
                                  195, 87, 100, 22, IDC_EDIT_MAX);
        MakeControl(h, L"BUTTON", L"Так", WS_TABSTOP | BS_DEFPUSHBUTTON, 55, 130, 90, 28, IDOK);
        MakeControl(h, L"BUTTON", L"Скасувати", WS_TABSTOP | BS_PUSHBUTTON, 165, 130, 90, 28, IDCANCEL);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            double n, mn, mx;
            if (!ParseNumber(GetText(st->edit[0]), n) || n != (int)n || n < 1 || n > 10000) {
                MessageBoxW(h, L"n має бути цілим числом від 1 до 10000.", L"Помилка введення", MB_OK | MB_ICONWARNING);
                SetFocus(st->edit[0]);
                return 0;
            }
            if (!ParseNumber(GetText(st->edit[1]), mn) || !ParseNumber(GetText(st->edit[2]), mx)) {
                MessageBoxW(h, L"Min і Max мають бути числами.", L"Помилка введення", MB_OK | MB_ICONWARNING);
                return 0;
            }
            if (mn > mx) {
                MessageBoxW(h, L"Min не може бути більшим за Max.", L"Помилка введення", MB_OK | MB_ICONWARNING);
                return 0;
            }
            st->out->n = (int)n;
            st->out->minValue = mn;
            st->out->maxValue = mx;
            st->ok = true;
            DestroyWindow(h);
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(h);
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(h);
        return 0;

    case WM_DESTROY:
        if (st) st->done = true;
        return 0;
    }
    return DefWindowProcW(h, msg, wParam, lParam);
}

static bool InputDialog(HWND owner, GenerateParams& out) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = InputDlgProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = kDlgClass;
        if (!RegisterClassExW(&wc)) return false;
        registered = true;
    }

    DlgState st = { { NULL, NULL, NULL }, &out, false, false };

    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    DWORD exStyle = WS_EX_DLGMODALFRAME;
    RECT rc = { 0, 0, 310, 175 };
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);
    int w = rc.right - rc.left, hgt = rc.bottom - rc.top;

    RECT ro;
    GetWindowRect(owner, &ro);
    int x = ro.left + ((ro.right - ro.left) - w) / 2;
    int y = ro.top + ((ro.bottom - ro.top) - hgt) / 2;

    HWND dlg = CreateWindowExW(exStyle, kDlgClass, L"Параметри генерації вектора",
        style, x, y, w, hgt, owner, NULL, GetModuleHandleW(NULL), &st);
    if (!dlg) return false;

    EnableWindow(owner, FALSE);
    ShowWindow(dlg, SW_SHOW);
    SetFocus(st.edit[0]);

    MSG msg;
    while (!st.done && GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageW(dlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    EnableWindow(owner, TRUE);
    SetForegroundWindow(owner);
    return st.ok;
}

class MainWindow {
    enum State { IDLE, WAIT_OBJECT2, WAIT_OBJECT3 };

    HWND      hWnd;
    HINSTANCE hInst;
    HWND      hList;
    State     state;

    void Log(const std::wstring& text) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t ts[32];
        wsprintfW(ts, L"[%02d:%02d:%02d]  ", st.wHour, st.wMinute, st.wSecond);
        std::wstring line = std::wstring(ts) + text;
        int idx = (int)SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());

        RECT rc;
        GetClientRect(hList, &rc);
        int itemH = (int)SendMessageW(hList, LB_GETITEMHEIGHT, 0, 0);
        int visible = itemH > 0 ? rc.bottom / itemH : 1;
        SendMessageW(hList, LB_SETTOPINDEX, idx + 1 > visible ? idx + 1 - visible : 0, 0);
    }

    void SetState(State s) {
        state = s;
        const wchar_t* names[] = { L"очікування", L"очікування відповіді Object2", L"очікування відповіді Object3" };
        std::wstring title = std::wstring(L"Lab6 - менеджер [") + names[s] + L"]";
        SetWindowTextW(hWnd, title.c_str());
    }

    HMENU CreateAppMenu() {
        HMENU hMenu = CreateMenu();
        HMENU hFile = CreatePopupMenu();
        HMENU hHelp = CreatePopupMenu();

        AppendMenuW(hFile, MF_STRING, IDM_RUN, L"Виконати...");
        AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hFile, MF_STRING, IDM_EXIT, L"Вихід");
        AppendMenuW(hHelp, MF_STRING, IDM_ABOUT, L"Про програму");

        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, L"Файл");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"Довідка");
        return hMenu;
    }

    HWND EnsureProgram(const wchar_t* cls, const wchar_t* exe) {
        HWND h = FindWindowW(cls, NULL);
        if (h) {
            Log(std::wstring(exe) + L" вже запущено - використовую наявне вікно");
            return h;
        }

        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        wchar_t* slash = wcsrchr(path, L'\\');
        if (slash) *(slash + 1) = 0;
        std::wstring full = std::wstring(path) + exe;

        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        if (!CreateProcessW(full.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            Log(std::wstring(L"ПОМИЛКА: не вдалося запустити ") + exe);
            return NULL;
        }
        Log(std::wstring(L"Запущено ") + exe);
        WaitForInputIdle(pi.hProcess, 5000);
        for (int i = 0; i < 100 && !h; i++) {
            h = FindWindowW(cls, NULL);
            if (!h) Sleep(50);
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        if (!h) Log(std::wstring(L"ПОМИЛКА: вікно ") + exe + L" не з'явилося");
        return h;
    }

    void Place(HWND h, int x, int y, int w, int hgt) {
        if (!h) return;
        if (IsIconic(h) || IsZoomed(h)) ShowWindow(h, SW_RESTORE);
        SetWindowPos(h, HWND_TOP, x, y, w, hgt, SWP_SHOWWINDOW);
    }

    void ArrangeWindows() {
        RECT wa;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
        int W = wa.right - wa.left;
        int H = wa.bottom - wa.top;
        int topH = H * 68 / 100;

        Place(FindWindowW(CLS_OBJECT2, NULL), wa.left, wa.top, W / 2, topH);
        Place(FindWindowW(CLS_OBJECT3, NULL), wa.left + W / 2, wa.top, W - W / 2, topH);
        Place(hWnd, wa.left, wa.top + topH, W, H - topH);
        SetForegroundWindow(hWnd);
    }

    void StartRun() {
        if (state != IDLE) {
            Log(L"Попередній сеанс ще виконується");
            return;
        }
        GenerateParams p;
        if (!InputDialog(hWnd, p)) return;

        Log(L"Параметри: n = " + IntW(p.n) + L", Min = " + NumW(p.minValue) +
            L", Max = " + NumW(p.maxValue));

        HWND obj2 = EnsureProgram(CLS_OBJECT2, EXE_OBJECT2);
        if (!obj2) return;

        SetState(WAIT_OBJECT2);
        SetTimer(hWnd, 1, 10000, NULL);

        COPYDATASTRUCT cds;
        cds.dwData = COPYDATA_GENERATE;
        cds.cbData = sizeof(p);
        cds.lpData = &p;
        Log(L"Lab6 -> Object2: WM_COPYDATA (команда генерації вектора)");
        SendMessageW(obj2, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&cds);
    }

    LRESULT OnDataReady(int n) {
        if (state != WAIT_OBJECT2) return 0;
        Log(L"Object2 -> Lab6: DataReady (вектор з " + IntW(n) +
            L" чисел записано в буфер обміну)");

        SetState(WAIT_OBJECT3);
        HWND obj3 = EnsureProgram(CLS_OBJECT3, EXE_OBJECT3);
        if (!obj3) {
            KillTimer(hWnd, 1);
            SetState(IDLE);
            return 0;
        }
        Log(L"Lab6 -> Object3: ShowGraph (прочитати буфер обміну і побудувати графік)");
        PostMessageW(obj3, MsgShowGraph(), (WPARAM)hWnd, 0);
        return 0;
    }

    LRESULT OnGraphDone(int count, int error) {
        if (state != WAIT_OBJECT3) return 0;
        KillTimer(hWnd, 1);
        if (error)
            Log(L"Object3 -> Lab6: GraphDone (ПОМИЛКА читання буфера обміну)");
        else
            Log(L"Object3 -> Lab6: GraphDone (побудовано графік з " + IntW(count) + L" точок)");
        ArrangeWindows();
        SetState(IDLE);
        Log(L"Сеанс завершено. Lab6 готова до повторного виконання");
        return 0;
    }

    LRESULT OnTimer() {
        KillTimer(hWnd, 1);
        if (state != IDLE) {
            Log(L"ПОМИЛКА: тайм-аут очікування відповіді");
            SetState(IDLE);
        }
        return 0;
    }

    LRESULT OnCreate() {
        SetMenu(hWnd, CreateAppMenu());
        hList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOSEL,
            0, 0, 0, 0, hWnd, NULL, hInst, NULL);
        SendMessageW(hList, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SetState(IDLE);
        Log(L"Lab6 запущено. Оберіть Файл -> Виконати...");
        return 0;
    }

    LRESULT OnCommand(WPARAM wParam) {
        switch (LOWORD(wParam)) {
        case IDM_RUN:
            StartRun();
            break;
        case IDM_ABOUT:
            MessageBoxW(hWnd,
                L"Лабораторна робота №6\nВаріант №11 (11 mod 4 = 3)\nВиконала: студентка Дрига Олександра ІМ-54",
                L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        return 0;
    }

    LRESULT OnDestroy() {
        KillTimer(hWnd, 1);
        HWND h2 = FindWindowW(CLS_OBJECT2, NULL);
        HWND h3 = FindWindowW(CLS_OBJECT3, NULL);
        if (h2) PostMessageW(h2, WM_CLOSE, 0, 0);
        if (h3) PostMessageW(h3, WM_CLOSE, 0, 0);
        PostQuitMessage(0);
        return 0;
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == MsgDataReady()) return OnDataReady((int)wParam);
        if (msg == MsgGraphDone()) return OnGraphDone((int)wParam, (int)lParam);

        switch (msg) {
        case WM_CREATE:  return OnCreate();
        case WM_SIZE:
            MoveWindow(hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            return 0;
        case WM_COMMAND: return OnCommand(wParam);
        case WM_TIMER:   return OnTimer();
        case WM_DESTROY: return OnDestroy();
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
    MainWindow(HINSTANCE h) : hWnd(NULL), hInst(h), hList(NULL), state(IDLE) {}

    bool Create(int nCmdShow) {
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = CLS_LAB6;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, CLS_LAB6, L"Lab6 - менеджер",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, 760, 300,
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