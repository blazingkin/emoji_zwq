#include <stdint.h>
#include <stdbool.h>
#include <cz/string.hpp>

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
    enum {Path, Circle} Kind;
    Color TheColor;
    union ShapeSpec {
        struct Path {
            String ThePath;
        } path;
        struct Circle {
            Point center;
            float radius;
        } circle;
    } Spec;
};

struct Svg {
    Vector <SvgElement> Elements;
    Point  Start;
    Point  Bounds;
};

bool ReadSVGFile(cz::Str path, Svg *Result);
