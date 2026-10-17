#ifndef SHAPES_H
#define SHAPES_H

#include "Shape.h"

class PointShape : public Shape {
public:
    void Show(HDC hdc) override {
        SetPixel(hdc, xs1, ys1, RGB(0, 0, 0));
        MoveToEx(hdc, xs1 - 1, ys1, NULL); LineTo(hdc, xs1 + 2, ys1);
        MoveToEx(hdc, xs1, ys1 - 1, NULL); LineTo(hdc, xs1, ys1 + 2);
    }
};

class LineShape : public Shape {
public:
    void Show(HDC hdc) override {
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        MoveToEx(hdc, xs1, ys1, NULL);
        LineTo(hdc, xs2, ys2);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
};

class RectangleShape : public Shape {
public:
    void Show(HDC hdc) override {
        long left   = std::min(xs1, xs2);
        long top    = std::min(ys1, ys2);
        long right  = std::max(xs1, xs2);
        long bottom = std::max(ys1, ys2);

        HPEN   hPen   = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0));   

        HPEN   hOldPen   = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, left, top, right, bottom);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        DeleteObject(hBrush);
    }
};

class EllipseShape : public Shape {
public:
    void Show(HDC hdc) override {
        long dx = std::abs(xs2 - xs1);
        long dy = std::abs(ys2 - ys1);

        HPEN   hPen      = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN   hOldPen   = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Ellipse(hdc, xs1 - dx, ys1 - dy, xs1 + dx, ys1 + dy);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
};

#endif