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
    bool     selected;
    int      typeId;

    HPEN MakePen() const {
        if (rubber)   return CreatePen(PS_DOT, 1, RGB(0, 0, 0));
        if (selected) return CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        return CreatePen(PS_SOLID, 1, penColor);
    }

public:
    Shape() : xs1(0), ys1(0), xs2(0), ys2(0), penColor(RGB(0, 0, 0)),
              rubber(false), selected(false), typeId(0) {}
    virtual ~Shape() {}

    virtual void Set(long x1, long y1, long x2, long y2) {
        xs1 = x1; ys1 = y1;
        xs2 = x2; ys2 = y2;
    }

    void GetCoords(long& x1, long& y1, long& x2, long& y2) const {
        x1 = xs1; y1 = ys1; x2 = xs2; y2 = ys2;
    }

    void SetRubber(bool r)   { rubber = r; }
    void SetSelected(bool s) { selected = s; }
    bool IsSelected() const  { return selected; }
    void SetTypeId(int id)   { typeId = id; }
    int  GetTypeId() const   { return typeId; }

    virtual void Show(HDC hdc) = 0;
};

#endif
