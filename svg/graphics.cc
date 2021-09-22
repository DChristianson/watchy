#include "graphics.h"

#include <iostream>

namespace wpp = watchpanel; 

wpp::Graphic::~Graphic() {}

wpp::TextGraphic::TextGraphic(
    const char * text,
    const std::map<std::string, std::string> & vars,
    const rgbmatrix::Font * font,
    rgbmatrix::Color color,
    int x,
    int y,
    int letter_spacing,
    int line_offset
) : text(text, vars),
    font(font), 
    color(color), 
    x(x), 
    y(y), 
    letter_spacing(letter_spacing), 
    line_offset(line_offset)
{}

void wpp::TextGraphic::Draw(rgbmatrix::Canvas *canvas)
{
    std::cout << "rgb_matrix::DrawText(offscreen, font," << x << ", " << y
     << " + font.baseline() + " << line_offset << ", color, NULL, " 
     <<  text.Value() << ", " << letter_spacing << ");" << std::endl;
    std::cout << "TextGraphic" << std::endl;
}

wpp::TextGraphic::~TextGraphic() {
}

wpp::ImageGraphic::ImageGraphic() {}

void wpp::ImageGraphic::Draw(rgbmatrix::Canvas *canvas)
{
    std::cout << "ImageGraphic" << std::endl;
}

wpp::ImageGraphic::~ImageGraphic() {
}

wpp::RectGraphic::RectGraphic() {}

void wpp::RectGraphic::Draw(rgbmatrix::Canvas *canvas)
{
    std::cout << "RectGraphic" << std::endl;
}

wpp::RectGraphic::~RectGraphic() {}
