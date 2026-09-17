#include "utils.h"

std::wstring g_MainDisplayText = L"Оберіть завдання в меню (Робота 1 або Робота 2)";

void RequestMainWindowUpdate(HWND hwndParent) {
    InvalidateRect(hwndParent, NULL, TRUE);
    UpdateWindow(hwndParent);
}