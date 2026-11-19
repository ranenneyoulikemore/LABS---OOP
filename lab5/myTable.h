#ifndef MYTABLE_H
#define MYTABLE_H

#include <windows.h>
#include <string>
#include <vector>

class MyTable {
public:
    struct Row {
        std::wstring name;
        long x1, y1, x2, y2;
    };

    class Listener {
    public:
        virtual ~Listener() {}
        virtual void OnRowSelected(int index) = 0;
        virtual void OnRowDeleteRequested(int index) = 0;
        virtual void OnTableClosed() = 0;
    };

    MyTable();
    ~MyTable();
    MyTable(const MyTable&) = delete;
    MyTable& operator=(const MyTable&) = delete;

    void SetListener(Listener* l) { listener = l; }

    bool Open(HWND owner, HINSTANCE hInst);
    void Close();
    bool IsOpen() const { return hwnd != NULL; }

    void SetRows(const std::vector<Row>& newRows);

private:
    HWND      hwnd;
    HWND      hList;
    HINSTANCE hInst;
    Listener* listener;
    std::vector<Row> rows;
    bool      updating;

    void Fill();
    LRESULT HandleMessage(HWND h, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif