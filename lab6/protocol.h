#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <windows.h>
#include <string>

#define CLS_LAB6      L"Lab6MainWindow"
#define CLS_OBJECT2   L"Lab6Object2Window"
#define CLS_OBJECT3   L"Lab6Object3Window"

#define EXE_OBJECT2   L"Object2.exe"
#define EXE_OBJECT3   L"Object3.exe"

#define COPYDATA_GENERATE 1

struct GenerateParams {
    int    n;
    double minValue;
    double maxValue;
};

inline std::wstring IntW(long long v) {
    wchar_t b[32];
    wsprintfW(b, L"%d", (int)v);
    return b;
}

inline UINT MsgDataReady() {
    static UINT m = RegisterWindowMessageW(L"Lab6.DataReady");
    return m;
}

inline UINT MsgShowGraph() {
    static UINT m = RegisterWindowMessageW(L"Lab6.ShowGraph");
    return m;
}

inline UINT MsgGraphDone() {
    static UINT m = RegisterWindowMessageW(L"Lab6.GraphDone");
    return m;
}

#endif