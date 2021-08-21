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

struct Tag {
    CloseType closetype;
    String name;
    Vector<Str> keys;
    Vector<Str> values;
};

typedef struct {

} Svg;

bool ReadSVGFile(cz::Str path, Svg *Result);
