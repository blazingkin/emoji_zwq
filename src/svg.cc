#include "svg.hpp"
#include <stdio.h>
#include <assert.h>
#include <cz/buffer_array.hpp>
#include "xml.hpp"
#include <iostream>

using namespace cz;



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
    printf("test\n");
    if (svgFile == NULL)
        return false;
    printf("Read\n");
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
                        Element.Spec.path.ThePath = pair.value.clone_null_terminate(alloc);
                    } else if (pair.key == "cx") {
                        if (Element.Kind == SvgElement::Ellipse) {
                            Element.Spec.ellipse.center.x = atof(pair.value.buffer);
                            printf("Ellipse center x: %f\n", Element.Spec.ellipse.center.x);
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
            std::cout << "TAG" << std::endl;
            

        } else {
            std::cout << "TEXT" << std::endl;
            std::cout << result.v.text.clone_null_terminate(alloc).buffer() << std::endl;
        }
    }
    printf("%d\n", ret);


    return true;
}