#ifndef SHAPEREGISTRY_H
#define SHAPEREGISTRY_H

#include <vector>
#include <cstring>
#include "Shapes.h"
#include "Ids.h"

struct ShapeInfo {
    int           id;
    const wchar_t* title;
    const char*   csvName;
    Shape*      (*create)();
};

class ShapeRegistry {
    template <class T> static Shape* Make() { return new T(); }

    static const std::vector<ShapeInfo>& Table() {
        static const std::vector<ShapeInfo> t = {
            { IDM_POINT,      L"Крапка",             "Point",       &Make<PointShape>     },
            { IDM_LINE,       L"Лінія",              "Line",        &Make<LineShape>      },
            { IDM_RECT,       L"Прямокутник",        "Rectangle",   &Make<RectangleShape> },
            { IDM_ELLIPSE,    L"Еліпс",              "Ellipse",     &Make<EllipseShape>   },
            { IDM_LINECIRCLE, L"Лінія з кружечками", "LineCircles", &Make<LineWithCircles> },
            { IDM_CUBE,       L"Каркас куба",        "Cube",        &Make<CubeShape>      },
        };
        return t;
    }

public:
    static const std::vector<ShapeInfo>& All() { return Table(); }

    static const ShapeInfo* ById(int id) {
        for (const ShapeInfo& i : Table()) if (i.id == id) return &i;
        return nullptr;
    }

    static const ShapeInfo* ByCsv(const char* name) {
        for (const ShapeInfo& i : Table()) if (std::strcmp(i.csvName, name) == 0) return &i;
        return nullptr;
    }

    static Shape* Create(int id) {
        const ShapeInfo* i = ById(id);
        if (!i) return nullptr;
        Shape* s = i->create();
        s->SetTypeId(id);
        return s;
    }
};

#endif