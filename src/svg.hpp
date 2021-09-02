#include <stdint.h>
#include <stdbool.h>
#include <cz/string.hpp>
#include <cz/heap.hpp>

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
        output->reserve(cz::heap_allocator(), 0);
    }
};

bool ReadSVGFile(const char* path, Svg *Result);
