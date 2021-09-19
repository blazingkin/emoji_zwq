#include "svg.hpp"
#include <stdio.h>
#include <assert.h>
#include <cz/buffer_array.hpp>
#include "xml.hpp"
#include <iostream>

using namespace cz;


Vector<PathElement> ParsePathElement(String input);
char buffer[0x200000] = {0};

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

    
bool ReadSVGFile(const char *path, Svg *OutputSvg)
// Read / Parse an SVG file and return the result in 
// *Result.
// returns whether or not the operation was successful.
{
    FILE *svgFile = fopen(path, "r");
    if (svgFile == NULL)
        return false;
    size_t size = fread(&buffer, sizeof(char), sizeof(buffer), svgFile);
    if (size == sizeof(buffer))
        assert("File too big!");

    cz::Str TheContents = Str(&buffer[0], size);
    size_t index = 0;
    cz::Buffer_Array TheAlloc = {};
    TheAlloc.init();
    cz::Allocator alloc = TheAlloc.allocator();

    cz::Buffer_Array NameAlloc = {};
    NameAlloc.init();
    cz::Allocator nameAlloc = NameAlloc.allocator();
    xml::Token result = {};
    xml::Result ret = xml::Result::Success;
    OutputSvg->Elements.reserve(alloc, 0x1000);
    while (ret != xml::Result::EndOfFile) {
        ret = xml::next_token(&index, TheContents, alloc, &result);
        if (result.tag == xml::Token::Tag) {
            if (result.v.tag.type == xml::Tag_Token::Close)
                continue;

            String Name = result.v.tag.name.clone_null_terminate(nameAlloc);
            if (Name == "svg") {
                for (auto pair: result.v.tag.pairs) {
                    if (pair.key == "viewBox") {
                        Vector<Str> values = {};
                        pair.value.split_into(' ', alloc, &values);
                        assert(values.len() == 4);
                        OutputSvg->Start = {(float)atoi(values[0].buffer),
                                            (float)atoi(values[1].buffer)};
                        OutputSvg->Bounds = {(float)atoi(values[2].buffer),
                                             (float)atoi(values[3].buffer)};
                    }
                }
            } else {
                SvgElement Element = {};
                if (Name == "path") {
                    Element.Kind = SvgElement::Path;
                } else if (Name == "circle") {
                    Element.Kind = SvgElement::Circle;
                } else if (Name == "ellipse") {
                    Element.Kind = SvgElement::Ellipse;
                } else {
                    std::cout << Name.buffer() << std::endl;
                    assert(false);
                }
                for (auto pair: result.v.tag.pairs) {
                    if (pair.key == "fill") {
                        // skip the #
                        uint32_t color = strtol(&pair.value.buffer[1], NULL, 16);
                        Element.TheColor.b = color & 0xFF;
                        Element.TheColor.g = (color >> 8) & 0xFF;
                        Element.TheColor.r = (color >> 16) & 0xFF;
                    } else if (pair.key == "d") {
                        Element.Spec.path.ThePath = ParsePathElement(pair.value.clone_null_terminate(alloc));
                    } else if (pair.key == "cx") {
                        if (Element.Kind == SvgElement::Ellipse) {
                            Element.Spec.ellipse.center.x = atof(pair.value.buffer);
                        } else {
                            assert(Element.Kind == SvgElement::Circle);
                            Element.Spec.circle.center.x = atof(pair.value.buffer);
                        }
                    } else if (pair.key == "cy") {
                        if (Element.Kind == SvgElement::Ellipse) {
                            Element.Spec.ellipse.center.y = atof(pair.value.buffer);
                        } else {
                            assert(Element.Kind == SvgElement::Circle);
                            Element.Spec.circle.center.y = atof(pair.value.buffer);
                        }
                    } else if (pair.key == "rx") {
                        assert(Element.Kind == SvgElement::Ellipse);
                        Element.Spec.ellipse.Radii.x = atof(pair.value.buffer);
                    } else if (pair.key == "ry") {
                        assert(Element.Kind == SvgElement::Ellipse);
                        Element.Spec.ellipse.Radii.y = atof(pair.value.buffer);
                    }
                }
                OutputSvg->Elements.push(Element);
            }
            

        } else {
            std::cout << result.v.text.clone_null_terminate(alloc).buffer() << std::endl;
        }
    }
    return true;
}

Vector<float> getPathNumbers(String input, size_t start, size_t expectedCount, size_t *last) {
    Vector<float> result = {};
    result.reserve(heap_allocator(), expectedCount);
    while (result.len() < expectedCount) {
        size_t index = start;
        while (isblank(input.buffer()[index])) {
            index++;
        }
        bool saw_decimal = false;
        bool should_continue = true;
        // look for the end of the number
        do {
            char c = input.buffer()[index];
            if (c == '.') {
                if (saw_decimal)
                    should_continue = false;
                saw_decimal = true;
            } else if (c == '-') {
                if (index != start)
                    should_continue = false;
            } else if (!isdigit(c)) {
                should_continue  = false;
            }
            if (should_continue)
                index++;
        } while (should_continue);
        index--; // We got one too many characters, remove the last
        result.push(atof(&input.buffer()[start]));

        // For the next number, continue on the next character
        start = index + 1;
    }
    *last = start - 1;
    return result;
}

Vector<PathElement> ParsePathElement(String input) {
    Vector<PathElement> result = {};
    result.reserve(heap_allocator(), 1000);
    Point cursorPosition = {0.0f,0.0f};
    Point shapePosition  = {0.0f,0.0f};
    Vector<float> nums   = {};
    char last = '0';
    for (size_t i = 0; i < input.len(); i++) {
        bool relative = false;
        PathElement NewElement = {};
        if (isalpha(input.buffer()[i])) {
            last = input.buffer()[i];
            i++;
        }
        switch (last) {
            // Move the cursor
            case 'm':
                relative = true;
            case 'M':
                nums = getPathNumbers(input, i, 2, &i);
                NewElement.Kind = PathElement::MoveTo;
                if (relative)
                    cursorPosition = cursorPosition + Point{nums[0], nums[1]};
                else
                    cursorPosition = Point{nums[0], nums[1]};
                shapePosition                = cursorPosition;
                NewElement.Spec.move.theMove = cursorPosition;
            break;
            
            // Line
            case 'l':
                relative = true;
            case 'L':
                nums = getPathNumbers(input, i, 2, &i);
                NewElement.Kind = PathElement::Line;
                NewElement.Spec.line.start = cursorPosition;
                if (relative)
                    cursorPosition = cursorPosition + Point{nums[0], nums[1]};
                else
                    cursorPosition = Point{nums[0], nums[1]};
                NewElement.Spec.line.end   = cursorPosition;
            break;

            // Vertical Line
            case 'v':
                relative = true;
            case 'V':
                nums = getPathNumbers(input, i, 1, &i);
                NewElement.Kind = PathElement::Line;
                NewElement.Spec.line.start = cursorPosition;
                if (relative)
                    cursorPosition = cursorPosition + Point{0, nums[0]};
                else
                    cursorPosition.y = nums[0];
                NewElement.Spec.line.end   = cursorPosition;
            break;

            // Horizontal line
            case 'h':
                relative = true;
            case 'H':
                nums = getPathNumbers(input, i, 1, &i);
                NewElement.Kind = PathElement::Line;
                NewElement.Spec.line.start = cursorPosition;
                if (relative)
                    cursorPosition = cursorPosition + Point{nums[0], 0};
                else
                    cursorPosition.x = nums[0];
                NewElement.Spec.line.end   = cursorPosition;
            break;

            // Complete shape
            case 'z':
            case 'Z':
                NewElement.Kind = PathElement::Line;
                NewElement.Spec.line.start = cursorPosition;
                NewElement.Spec.line.end   = shapePosition;
                cursorPosition = shapePosition;
            break;

            // Bezier curve
            case 'c':
                relative = true;
            case 'C':
                nums = getPathNumbers(input, i, 6, &i);
                NewElement.Kind = PathElement::BezierCurve;
                NewElement.Spec.curve.degree = 3;
                NewElement.Spec.curve.first  = Point{nums[0], nums[1]};
                NewElement.Spec.curve.second = Point{nums[2], nums[3]};
                NewElement.Spec.curve.third  = Point{nums[4], nums[5]};
                if (relative) {
                    NewElement.Spec.curve.first  = cursorPosition + NewElement.Spec.curve.first;
                    NewElement.Spec.curve.second = cursorPosition + NewElement.Spec.curve.second;
                    NewElement.Spec.curve.third  = cursorPosition + NewElement.Spec.curve.third;
                }
                cursorPosition = NewElement.Spec.curve.third;
            break;

            // Reflection Bezier curve
            case 's':
                relative = true;
            case 'S':
                nums = getPathNumbers(input, i, 4, &i);
                NewElement.Kind = PathElement::BezierCurve;
                NewElement.Spec.curve.degree = 2;
                NewElement.Spec.curve.first  = Point{nums[0], nums[1]};
                NewElement.Spec.curve.second = Point{nums[2], nums[3]};
                if (relative) {
                    NewElement.Spec.curve.first  = cursorPosition + NewElement.Spec.curve.first;
                    NewElement.Spec.curve.second = cursorPosition + NewElement.Spec.curve.second;
                    NewElement.relative = true;
                }
                cursorPosition = NewElement.Spec.curve.second;
            break;

            default:
                if (isalpha(input.buffer()[i]))
                    printf("Unhandled %c\n", input.buffer()[i]);
        }
        result.push(NewElement);
    }

    return result;
}