#include <stdint.h>
#include <stdbool.h>
#include <cz/string.hpp>
#include <cz/heap.hpp>
#include <cstdio>
using namespace cz;

enum CloseType {
    invalid,
    selfclosing,
    open,
    close
};

struct Point {
    float x;
    float y;

    Point operator+(const Point& other) const {
        return Point{x + other.x, y + other.y};
    }

};

struct Color {
    int r, g, b;
};


struct SvgElement {
    enum {Path, Circle, Ellipse} Kind;
    Color TheColor;
    union ShapeSpec {
        struct Path {
            String ThePath;
        } path;
        struct Circle {
            Point center;
            float radius;
        } circle;
        struct Ellipse {
            Point center;
            Point Radii;
        } ellipse;
    } Spec;

    void translate(Point p) {
        switch (Kind) {
            case Path:
            break;
            case Circle:
                Spec.circle.center = Spec.circle.center + p;
            break;
            case Ellipse:
                Spec.ellipse.center = Spec.ellipse.center + p; 
            break;
        }
    }

    void output(String *out) {
        char buffer[0x40] = {0};
        char colorBuffer[0x40] = {0};

        snprintf((char *)colorBuffer, 0x40, " fill=\"#%02X%02X%02X\" ", TheColor.r, TheColor.g, TheColor.b);

        switch (Kind) {
            case Path:
                out->append({"<path "});
                out->append({colorBuffer});
                out->append({"d=\""});
                out->append(Spec.path.ThePath.as_str());
                out->push('"');
                out->push('/');
                out->push('>');
            break;
            case Circle:
                out->append({"<circle "});
                out->append({colorBuffer});
                snprintf((char *) buffer, 0x40, "cx=\"%f\" cy=\"%f\" r=\"%f\"", 
                         Spec.circle.center.x, Spec.circle.center.y,
                         Spec.circle.radius);
                out->append({buffer});
                out->push('/');
                out->push('>');
            break;
            case Ellipse:
                out->append({"<ellipse "});
                out->append({colorBuffer});
                snprintf((char *) buffer, 0x40, "cx=\"%f\" cy=\"%f\" rx=\"%f\" ry=\"%f\"", 
                         Spec.ellipse.center.x, Spec.ellipse.center.y,
                         Spec.ellipse.Radii.x,  Spec.ellipse.Radii.y);
                out->append({buffer});
                out->push('/');
                out->push('>');
            break;
        }

    }
};

struct Svg {
    Vector <SvgElement> Elements;
    Point  Start;
    Point  Bounds;

    void translate(Point p) {
        for (auto element: Elements) {
            element.translate(p);
        }
    }

    void output(String *output) {
        char numBuf[0x40] = {0};
        output->reserve(cz::heap_allocator(), 0x10000);
        output->append(cz::Str{"<svg xmlns=\"http://www.w3.org/2000/svg\" viewbox=\""});
        snprintf((char *)&numBuf, 0x40, "%0.0f %0.0f %0.0f %0.0f", Start.x, Start.y, Bounds.x, Bounds.y);

        output->append(cz::Str{numBuf});
        output->append(cz::Str{"\">"});

        for (auto elem : Elements) {
            elem.output(output);
        }

        output->append(cz::Str{"</svg>"});
    }

    void insert(Svg &Other) {
        for (auto elem : Other.Elements) {
            Elements.push(elem);
        }
        Start.x  = min(Start.x,  Other.Start.x);
        Start.y  = min(Start.y,  Other.Start.y);

        Bounds.x = max(Bounds.x, Other.Bounds.x);
        Bounds.y = max(Bounds.y, Other.Bounds.y);
    }
};

bool ReadSVGFile(const char* path, Svg *Result);
