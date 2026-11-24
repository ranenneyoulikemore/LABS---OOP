#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <vector>
#include "MyEditor.h"
#include "MyTable.h"
#include "Toolbar.h"

#ifdef _MSC_VER
#pragma comment(lib, "comdlg32.lib")
#endif

class MainWindow {
    HWND      hWnd;
    HINSTANCE hInst;
    HMENU     hObjectsMenu;
    Toolbar   toolbar;
    MyTable   table;
    MyEditor& editor;

    HMENU CreateAppMenu() {
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        hObjectsMenu = CreatePopupMenu();
        HMENU hWindowMenu = CreatePopupMenu();
        HMENU hHelpMenu = CreatePopupMenu();

        AppendMenuW(hFileMenu, MF_STRING, IDM_OPEN, L"Відкрити...");
        AppendMenuW(hFileMenu, MF_STRING, IDM_SAVE, L"Зберегти як...");
        AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hFileMenu, MF_STRING, IDM_EXIT, L"Вихід");

        for (const ShapeInfo& info : ShapeRegistry::All())
            AppendMenuW(hObjectsMenu, MF_STRING, info.id, info.title);

        AppendMenuW(hWindowMenu, MF_STRING, IDM_TABLE, L"Таблиця об'єктів");

        AppendMenuW(hHelpMenu, MF_STRING, IDM_ABOUT, L"Про програму");

        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"Файл");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hObjectsMenu, L"Об'єкти");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hWindowMenu, L"Вікно");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"Довідка");
        return hMenu;
    }

    void RefreshTable() {
        if (!table.IsOpen()) return;
        std::vector<MyTable::Row> rows;
        for (int i = 0; i < editor.Count(); i++) {
            const Shape* s = editor.Get(i);
            const ShapeInfo* info = ShapeRegistry::ById(s->GetTypeId());
            MyTable::Row r;
            r.name = info ? info->title : L"?";
            s->GetCoords(r.x1, r.y1, r.x2, r.y2);
            rows.push_back(r);
        }
        table.SetRows(rows);
    }

    void UpdateTableMenu() {
        CheckMenuItem(GetMenu(hWnd), IDM_TABLE,
            MF_BYCOMMAND | (table.IsOpen() ? MF_CHECKED : MF_UNCHECKED));
    }

    void ToggleTable() {
        if (table.IsOpen()) {
            table.Close();
        }
        else if (table.Open(hWnd, hInst)) {
            RefreshTable();
        }
        UpdateTableMenu();
    }

    void InitTable() {
        MyTable::Listener listener;
        listener.onRowSelected = [this](int index) { editor.Select(index); };
        listener.onRowDeleteRequested = [this](int index) { editor.Remove(index); };
        listener.onClosed = [this]() {
            editor.Select(-1);
            UpdateTableMenu();
        };
        table.SetListener(listener);
    }

    bool PickFile(bool save, wchar_t* path) {
        OPENFILENAMEW ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFilter = L"CSV (*.csv)\0*.csv\0Усі файли (*.*)\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = L"csv";
        ofn.Flags = save ? (OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST)
                         : (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
        return save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    }

    void SaveFile() {
        wchar_t path[MAX_PATH] = L"shapes.csv";
        if (!PickFile(true, path)) return;
        if (!editor.Save(path))
            MessageBoxW(hWnd, L"Не вдалося записати файл.", L"Помилка", MB_OK | MB_ICONERROR);
    }

    void OpenFile() {
        wchar_t path[MAX_PATH] = L"";
        if (!PickFile(false, path)) return;
        if (editor.Load(path) < 0)
            MessageBoxW(hWnd, L"Не вдалося відкрити файл.", L"Помилка", MB_OK | MB_ICONERROR);
    }

    LRESULT OnCreate() {
        SetMenu(hWnd, CreateAppMenu());
        toolbar.Create(hWnd, hInst);
        editor.Attach(hWnd);
        editor.SetOnChanged([this]() { RefreshTable(); });
        InitTable();
        return 0;
    }

    LRESULT OnSize() {
        toolbar.AutoSize();
        return 0;
    }

    LRESULT OnCommand(WPARAM wParam) {
        int id = LOWORD(wParam);
        switch (id) {
        case IDM_OPEN:  OpenFile();    break;
        case IDM_SAVE:  SaveFile();    break;
        case IDM_TABLE: ToggleTable(); break;
        case IDM_ABOUT:
            MessageBoxW(hWnd,
                L"Лабораторна робота №5\nВаріант №11\nВиконала: студентка Дрига Олександра ІМ-54",
                L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            if (ShapeRegistry::ById(id)) {
                editor.SetType(id);
                toolbar.Check(id);
            }
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
            CheckMenuRadioItem(hMenu, ShapeRegistry::All().front().id,
                               ShapeRegistry::All().back().id,
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
        table.ClearListener();
        table.Close();
        editor.SetOnChanged(nullptr);
        editor.Attach(NULL);
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
        : hWnd(NULL), hInst(h), hObjectsMenu(NULL), editor(MyEditor::Instance()) {}

    bool Create(int nCmdShow) {
        const wchar_t cls[] = L"Lab5WindowClass";

        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = cls;
        if (!RegisterClassExW(&wc)) return false;

        HWND h = CreateWindowExW(0, cls, L"Lab5 - Графічний редактор",
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