#ifndef MYEDITOR_H
#define MYEDITOR_H

#include <cstdio>
#include <functional>
#include "ShapeRegistry.h"

class MyEditor {
    static const int N = 112;

    Shape** pcshape;
    int     shapeCount;
    int     currentType;
    bool    isDrawing;
    POINT   ptStart, ptCurrent;

    int     selected;
    HWND    hView;
    std::function<void()> onChanged;

    MyEditor() : shapeCount(0), currentType(IDM_POINT), isDrawing(false),
                 selected(-1), hView(NULL) {
        pcshape = new Shape * [N];
        for (int i = 0; i < N; i++) pcshape[i] = nullptr;
        ptStart.x = ptStart.y = ptCurrent.x = ptCurrent.y = 0;
    }

    ~MyEditor() {
        for (int i = 0; i < shapeCount; i++) delete pcshape[i];
        delete[] pcshape;
    }

    void Redraw() { if (hView) InvalidateRect(hView, NULL, TRUE); }
    void Notify() { if (onChanged) onChanged(); }

    void ClearSelection() {
        selected = -1;
        for (int i = 0; i < shapeCount; i++) pcshape[i]->SetSelected(false);
    }

    void DrawRubber(HWND hWnd, POINT p1, POINT p2) {
        Shape* s = ShapeRegistry::Create(currentType);
        if (!s) return;
        s->SetRubber(true);
        s->Set(p1.x, p1.y, p2.x, p2.y);

        HDC hdc = GetDC(hWnd);
        SetROP2(hdc, R2_NOTXORPEN);
        SetBkMode(hdc, TRANSPARENT);
        s->Show(hdc);
        ReleaseDC(hWnd, hdc);
        delete s;
    }

    void AddShape(Shape* s, POINT p1, POINT p2) {
        if (!s) return;
        if (shapeCount < N) {
            ClearSelection();
            s->Set(p1.x, p1.y, p2.x, p2.y);
            pcshape[shapeCount++] = s;
            Redraw();
            Notify();
        }
        else {
            delete s;
        }
    }

    void ClearShapes() {
        for (int i = 0; i < shapeCount; i++) { delete pcshape[i]; pcshape[i] = nullptr; }
        shapeCount = 0;
        selected = -1;
    }

public:
    static MyEditor& Instance() {
        static MyEditor instance;
        return instance;
    }

    MyEditor(const MyEditor&) = delete;
    MyEditor& operator=(const MyEditor&) = delete;

    void Attach(HWND h) { hView = h; }
    void SetOnChanged(std::function<void()> f) { onChanged = f; }

    void SetType(int type) { currentType = type; }
    int  GetType() const   { return currentType; }

    int          Count() const { return shapeCount; }
    const Shape* Get(int i) const {
        return (i >= 0 && i < shapeCount) ? pcshape[i] : nullptr;
    }

    void Select(int i) {
        if (i < 0 || i >= shapeCount) i = -1;
        selected = i;
        for (int k = 0; k < shapeCount; k++) pcshape[k]->SetSelected(k == i);
        Redraw();
    }

    void Remove(int i) {
        if (i < 0 || i >= shapeCount) return;
        delete pcshape[i];
        for (int k = i; k < shapeCount - 1; k++) pcshape[k] = pcshape[k + 1];
        pcshape[--shapeCount] = nullptr;
        ClearSelection();
        Redraw();
        Notify();
    }

    bool Save(const wchar_t* path) const {
        FILE* f = _wfopen(path, L"w");
        if (!f) return false;
        for (int i = 0; i < shapeCount; i++) {
            const ShapeInfo* info = ShapeRegistry::ById(pcshape[i]->GetTypeId());
            if (!info) continue;
            long x1, y1, x2, y2;
            pcshape[i]->GetCoords(x1, y1, x2, y2);
            fprintf(f, "%s,%ld,%ld,%ld,%ld\n", info->csvName, x1, y1, x2, y2);
        }
        fclose(f);
        return true;
    }

    int Load(const wchar_t* path) {
        FILE* f = _wfopen(path, L"r");
        if (!f) return -1;
        ClearShapes();

        char line[256];
        int  loaded = 0;
        while (fgets(line, sizeof(line), f)) {
            char name[32];
            long a, b, c, d;
            if (sscanf(line, "%31[^,],%ld,%ld,%ld,%ld", name, &a, &b, &c, &d) != 5) continue;
            const ShapeInfo* info = ShapeRegistry::ByCsv(name);
            if (!info) continue;
            if (shapeCount >= N) break;
            Shape* s = ShapeRegistry::Create(info->id);
            s->Set(a, b, c, d);
            pcshape[shapeCount++] = s;
            loaded++;
        }
        fclose(f);
        Redraw();
        Notify();
        return loaded;
    }

    void OnLButtonDown(HWND hWnd, POINT pt) {
        hView = hWnd;
        ptStart = pt;
        ptCurrent = pt;

        if (currentType == IDM_POINT) {
            AddShape(ShapeRegistry::Create(currentType), pt, pt);
            return;
        }
        isDrawing = true;
        SetCapture(hWnd);
        DrawRubber(hWnd, ptStart, ptCurrent);
    }

    void OnMouseMove(HWND hWnd, POINT pt) {
        if (!isDrawing) return;
        DrawRubber(hWnd, ptStart, ptCurrent);
        ptCurrent = pt;
        DrawRubber(hWnd, ptStart, ptCurrent);
    }

    void OnLButtonUp(HWND hWnd, POINT pt) {
        if (!isDrawing) return;
        DrawRubber(hWnd, ptStart, ptCurrent);
        isDrawing = false;
        ReleaseCapture();
        ptCurrent = pt;
        AddShape(ShapeRegistry::Create(currentType), ptStart, ptCurrent);
    }

    void OnPaint(HDC hdc) const {
        for (int i = 0; i < shapeCount; i++)
            if (pcshape[i]) pcshape[i]->Show(hdc);
    }
};

#endif