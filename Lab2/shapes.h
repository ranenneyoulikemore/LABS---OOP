#ifndef SHAPES_H
#define SHAPES_H

#include "Shape.h"
#include <cmath>

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
        long dx = std::abs(xs2 - xs1);
        long dy = std::abs(ys2 - ys1);

        long left = xs1 - dx;
        long top = ys1 - dy;
        long right = xs1 + dx;
        long bottom = ys1 + dy;

        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hBrush = CreateSolidBrush(RGB(128, 128, 128));

        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
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
        long left = min(xs1, xs2);
        long top = min(ys1, ys2);
        long right = max(xs1, xs2);
        long bottom = max(ys1, ys2);

        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);

        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Ellipse(hdc, left, top, right, bottom);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
};

#endif