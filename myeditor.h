#ifndef MYEDITOR_H
#define MYEDITOR_H

#include "Shapes.h"
#include "Ids.h"

class MyEditor {
    static const int N = 112;

    Shape** pcshape;
    int     shapeCount;
    int     currentType;
    bool    isDrawing;
    POINT   ptStart, ptCurrent;

    Shape* CreateShape(int type) const {
        switch (type) {
        case IDM_POINT:      return new PointShape();
        case IDM_LINE:       return new LineShape();
        case IDM_RECT:       return new RectangleShape();
        case IDM_ELLIPSE:    return new EllipseShape();
        case IDM_LINECIRCLE: return new LineWithCircles();
        case IDM_CUBE:       return new CubeShape();
        }
        return nullptr;
    }

    void DrawRubber(HWND hWnd, POINT p1, POINT p2) {
        Shape* s = CreateShape(currentType);
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

    void AddShape(HWND hWnd, Shape* s, POINT p1, POINT p2) {
        if (!s) return;
        if (shapeCount < N) {
            s->Set(p1.x, p1.y, p2.x, p2.y);
            pcshape[shapeCount++] = s;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else {
            delete s;
        }
    }

public:
    MyEditor() : shapeCount(0), currentType(IDM_POINT), isDrawing(false) {
        pcshape = new Shape * [N];
        for (int i = 0; i < N; i++) pcshape[i] = nullptr;
        ptStart.x = ptStart.y = ptCurrent.x = ptCurrent.y = 0;
    }

    ~MyEditor() {
        for (int i = 0; i < shapeCount; i++) delete pcshape[i];
        delete[] pcshape;
    }

    MyEditor(const MyEditor&) = delete;
    MyEditor& operator=(const MyEditor&) = delete;

    void SetType(int type) { currentType = type; }
    int  GetType() const   { return currentType; }

    void OnLButtonDown(HWND hWnd, POINT pt) {
        ptStart = pt;
        ptCurrent = pt;

        if (currentType == IDM_POINT) {
            AddShape(hWnd, CreateShape(currentType), pt, pt);
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
        AddShape(hWnd, CreateShape(currentType), ptStart, ptCurrent);
    }

    void OnPaint(HDC hdc) const {
        for (int i = 0; i < shapeCount; i++)
            if (pcshape[i]) pcshape[i]->Show(hdc);
    }
};

#endif