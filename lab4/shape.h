#ifndef SHAPE_H
#define SHAPE_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cstdlib>

class Shape {
protected:
    long     xs1, ys1, xs2, ys2;
    COLORREF penColor;
    bool     rubber;

    HPEN MakePen() const {
        return rubber ? CreatePen(PS_DOT, 1, RGB(0, 0, 0))
                      : CreatePen(PS_SOLID, 1, penColor);
    }

public:
    Shape() : xs1(0), ys1(0), xs2(0), ys2(0), penColor(RGB(0, 0, 0)), rubber(false) {}
    virtual ~Shape() {}

    virtual void Set(long x1, long y1, long x2, long y2) {
        xs1 = x1; ys1 = y1;
        xs2 = x2; ys2 = y2;
    }

    void SetRubber(bool r) { rubber = r; }

    virtual void Show(HDC hdc) = 0;
};

#endif