#ifndef MYTABLE_H
#define MYTABLE_H

#include <windows.h>
#include <functional>
#include <string>
#include <vector>

class MyTable {
public:
    struct Row {
        std::wstring name;
        long x1, y1, x2, y2;
    };

    struct Listener {
        std::function<void(int)> onRowSelected;
        std::function<void(int)> onRowDeleteRequested;
        std::function<void()>    onClosed;
    };

    MyTable();
    ~MyTable();
    MyTable(const MyTable&) = delete;
    MyTable& operator=(const MyTable&) = delete;

    void SetListener(const Listener& l) { listener = l; }
    void ClearListener() { listener = Listener(); }

    bool Open(HWND owner, HINSTANCE hInst);
    void Close();
    bool IsOpen() const { return hwnd != NULL; }

    void SetRows(const std::vector<Row>& newRows);

private:
    HWND      hwnd;
    HWND      hList;
    HINSTANCE hInst;
    Listener  listener;
    std::vector<Row> rows;
    bool      updating;

    void Fill();
    LRESULT HandleMessage(HWND h, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif