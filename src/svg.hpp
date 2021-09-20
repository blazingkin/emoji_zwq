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
    Point operator*(const Point& other) const {
        return Point{x * other.x, y * other.y};
    }
    Point operator*(const float& other) const {
        return Point{x * other, y * other};
    }

};

struct Color {
    int r, g, b;
};

struct PathElement {
    enum {MoveTo, Line, BezierCurve, Arc, Close} Kind;
    union ElementSpec {
        struct MoveTo{
            Point theMove;
        } move;
        struct Line {
            Point start;
            Point end;
        } line;
        struct Curve {
            int degree;
            Point first;
            Point second;
            Point third;
        } curve;
        struct Arc {
            Point Center;
            Point Radii;
            float rotation;
            bool  large_arc;
            bool  sweep;
        } arc;
    } Spec;
    bool relative;
    void output(String *out) {
        char buffer[0x40] = {0};
        switch (Kind) {
            case MoveTo:
                snprintf((char *) buffer, 0x40, "%c %f %f", relative ? 'm' : 'M',
                                                             Spec.move.theMove.x, Spec.move.theMove.y);
                out->append({buffer});
            break;
            case Line:
                snprintf((char *)buffer, 0x40, "%c %f %f", relative ? 'l' : 'L',
                                                           Spec.line.end.x, Spec.line.end.y);
                out->append({buffer});
            break;
            case BezierCurve:
                if (Spec.curve.degree == 3) {
                    snprintf((char *)buffer,0x40,"%c %f %f %f %f %f %f",relative ? 'c' : 'C',
                                                                        Spec.curve.first.x, Spec.curve.first.y,
                                                                        Spec.curve.second.x, Spec.curve.second.y,
                                                                        Spec.curve.third.x, Spec.curve.third.y);
                } else if (Spec.curve.degree == 2) {
                    snprintf((char *)buffer,0x40,"S %f %f %f %f", Spec.curve.first.x, Spec.curve.first.y,
                                                                        Spec.curve.second.x, Spec.curve.second.y);
                }
                out->append({buffer});
            break;
            case Close:
                out->push('Z');
            break;
            default:
            break;
        }
    }

    void translate(Point offset) {
        switch (Kind) {
            case MoveTo:
                Spec.move.theMove = Spec.move.theMove + offset;
            break;
            case Line:
                Spec.line.start = Spec.line.start + offset;
                Spec.line.end = Spec.line.end + offset;
            break;
            case BezierCurve:
                Spec.curve.first = Spec.curve.first + offset;
                Spec.curve.second = Spec.curve.second + offset;
                Spec.curve.third = Spec.curve.third + offset;
            break;
            default:
            break;
        }
    }

    void rescale(Point scale) {
        switch (Kind)
        {
        case MoveTo:
            Spec.move.theMove = Spec.move.theMove * scale; 
        break;
        case Line:
            Spec.line.start = Spec.line.start * scale;
            Spec.line.end   = Spec.line.end   * scale;
        break;
        case BezierCurve:
            Spec.curve.first  = Spec.curve.first  * scale;
            Spec.curve.second = Spec.curve.second * scale;
            Spec.curve.third  = Spec.curve.third  * scale;
        break;
        default:
        break;
        }
    }
};


struct SvgElement {
    enum {Path, Circle, Ellipse} Kind;
    Color TheColor;
    union ShapeSpec {
        struct Path {
            Vector<PathElement> ThePath;
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
        size_t i;
        switch (Kind) {
            case Path:
                for (i = 0; i < Spec.path.ThePath.len(); i++) {
                    Spec.path.ThePath[i].translate(p);
                }
            break;
            case Circle:
                Spec.circle.center = Spec.circle.center + p;
            break;
            case Ellipse:
                Spec.ellipse.center = Spec.ellipse.center + p; 
            break;
        }
    }

    void rescale(Point scale) {
        size_t i;
        switch (Kind) {
            case Path:
                for (i = 0; i < Spec.path.ThePath.len(); i++) {
                    Spec.path.ThePath[i].rescale(scale);
                }
            break;
            case Circle:
                Spec.circle.center = Spec.circle.center  * scale;
                Spec.circle.radius *= scale.x;
            break;
            case Ellipse:
                Spec.ellipse.center = Spec.ellipse.center * scale; 
                Spec.ellipse.Radii  = Spec.ellipse.Radii  * scale;
            break;
        }
    }

    void output(String *out) {
        char buffer[0x40] = {0};
        char colorBuffer[0x40] = {0};

        snprintf((char *)colorBuffer, 0x40, " fill=\"#%02X%02X%02X\" ", TheColor.r, TheColor.g, TheColor.b);

        switch (Kind) {
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
            case Path:
                out->append({"<path "});
                out->append({colorBuffer});
                out->append("d=\"");
                for (auto element: Spec.path.ThePath) {
                    element.output(out);
                }
                out->push('"');
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
        for (size_t i = 0; i < Elements.len(); i++) {
            Elements[i].translate(p);
        }
    }

    void rescale(Point scale) {
        for (size_t i = 0; i < Elements.len(); i++) {
            Elements[i].rescale(scale);
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
