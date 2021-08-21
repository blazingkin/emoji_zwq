#include <stdint.h>
#include <stdbool.h>
#include <cz/string.hpp>

using namespace cz;

typedef struct {
    double X;
    double y;
} Point;

typedef struct {
    char *Name;
    char *Value;
} Tag;

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} Color;

typedef struct {
    enum {
        Move,
        Line,
        CubicCurve,
        QuadraticCurve,
        Arc,
    } Type;
    Point End;
    Point StartControlPoint;
    Point EndControlPoint;
} SvgPathElement;

typedef struct {
    Point     Start;
    Color     Fill;
    Tag      *OtherTags;
    uint32_t  OtherTagsLength;
} SvgPath;

typedef struct {
    Point     ViewUpperLeft;
    Point     ViewBottomRight;
    Tag      *OtherTags;
    uint32_t  OtherTagsLength;
    SvgPath  *Paths;
    uint32_t  PathsLength;
} Svg;

bool ReadSVGFile(cz::String path, Svg *Result);
