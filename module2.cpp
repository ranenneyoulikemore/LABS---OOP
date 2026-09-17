#include "module2.h"
#include "utils.h"
#include "resource.h" 

static INT_PTR CALLBACK DlgProcV2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            WCHAR buffer[256];
            GetDlgItemText(hDlg, IDC_EDIT1, buffer, 256);
            g_MainDisplayText = L"Робота 2 (Введений текст): " + std::wstring(buffer);

            RequestMainWindowUpdate(GetParent(hDlg));
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void RunWork2(HINSTANCE hInst, HWND hWndParent) {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWndParent, DlgProcV2);
}