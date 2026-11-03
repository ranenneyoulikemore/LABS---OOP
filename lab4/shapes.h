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

class LineShape : public virtual Shape {
public:
    void Show(HDC hdc) override {
        HPEN hPen = MakePen();
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        MoveToEx(hdc, xs1, ys1, NULL);
        LineTo(hdc, xs2, ys2);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
};

class RectangleShape : public virtual Shape {
protected:
    bool     filled;
    COLORREF fillColor;
public:
    RectangleShape() : filled(true), fillColor(RGB(255, 255, 0)) {}

    void Show(HDC hdc) override {
        long left   = std::min(xs1, xs2);
        long top    = std::min(ys1, ys2);
        long right  = std::max(xs1, xs2);
        long bottom = std::max(ys1, ys2);

        bool useFill = filled && !rubber;
        HPEN   hPen   = MakePen();
        HBRUSH hBrush = useFill ? CreateSolidBrush(fillColor)
                                : (HBRUSH)GetStockObject(NULL_BRUSH);

        HPEN   hOldPen   = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, left, top, right, bottom);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        if (useFill) DeleteObject(hBrush);
    }
};

class EllipseShape : public virtual Shape {
protected:
    bool     filled;
    COLORREF fillColor;
public:
    EllipseShape() : filled(false), fillColor(RGB(255, 255, 255)) {}

    void Show(HDC hdc) override {
        long dx = std::abs(xs2 - xs1);
        long dy = std::abs(ys2 - ys1);

        bool useFill = filled && !rubber;
        HPEN   hPen   = MakePen();
        HBRUSH hBrush = useFill ? CreateSolidBrush(fillColor)
                                : (HBRUSH)GetStockObject(NULL_BRUSH);

        HPEN   hOldPen   = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Ellipse(hdc, xs1 - dx, ys1 - dy, xs1 + dx, ys1 + dy);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        if (useFill) DeleteObject(hBrush);
    }
};

class LineWithCircles : public LineShape, public EllipseShape {
public:
    LineWithCircles() {
        penColor  = RGB(200, 0, 0);
        filled    = true;
        fillColor = RGB(255, 200, 200);
    }

    void Show(HDC hdc) override {
        const long x1 = xs1, y1 = ys1, x2 = xs2, y2 = ys2;
        const long R = 6;

        LineShape::Show(hdc);

        Set(x1, y1, x1 + R, y1 + R);
        EllipseShape::Show(hdc);
        Set(x2, y2, x2 + R, y2 + R);
        EllipseShape::Show(hdc);

        Set(x1, y1, x2, y2);
    }
};

class CubeShape : public LineShape, public RectangleShape {
public:
    CubeShape() {
        penColor = RGB(0, 0, 200);
        filled   = false;
    }

    void Show(HDC hdc) override {
        const long x1 = xs1, y1 = ys1, x2 = xs2, y2 = ys2;
        long l = std::min(x1, x2), t = std::min(y1, y2);
        long r = std::max(x1, x2), b = std::max(y1, y2);
        long d = std::min(r - l, b - t) / 3;

        Set(l + d, t, r, b - d);   RectangleShape::Show(hdc);
        Set(l, t + d, r - d, b);   RectangleShape::Show(hdc);

        Set(l, t + d, l + d, t);       LineShape::Show(hdc);
        Set(r - d, t + d, r, t);       LineShape::Show(hdc);
        Set(l, b, l + d, b - d);       LineShape::Show(hdc);
        Set(r - d, b, r, b - d);       LineShape::Show(hdc);

        Set(x1, y1, x2, y2);
    }
};

#endif