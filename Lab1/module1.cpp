#include "module1.h"
#include "utils.h"
#include "resource.h" 

static INT_PTR CALLBACK DlgProcV1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        HWND hListBox = GetDlgItem(hDlg, IDC_LIST1);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"ІП-31");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"ІМ-36");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"ІА-55");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"ІМ-54");
        SendMessage(hListBox, LB_SETCURSEL, 0, 0);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            HWND hListBox = GetDlgItem(hDlg, IDC_LIST1);
            int itemIndex = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (itemIndex != LB_ERR) {
                WCHAR buffer[256];
                SendMessage(hListBox, LB_GETTEXT, itemIndex, (LPARAM)buffer);
                g_MainDisplayText = L"Робота 1 (Вибрана група): " + std::wstring(buffer);
                RequestMainWindowUpdate(GetParent(hDlg));
            }
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

void RunWork1(HINSTANCE hInst, HWND hWndParent) {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWndParent, DlgProcV1);
}