#include "watchpanel.h"
#include "pugixml.hpp"

#include <stdlib.h>
#include <iostream>

#include <fmt/args.h>
#include <fmt/core.h>

namespace watchpanel {

    const char * _PAGE_ = "page";
    const char * _DATA_ = "data";
    const char * _DISPLAY_ = "display";
    const char * _TEXT_ = "text";
    const char * _IMAGE_ = "image";
    const char * _RECT_ = "rect";
    const char * _FONT_ = "font";
    const char * _X_ = "x";
    const char * _Y_ = "y";
    const char * _WIDTH_ = "w";
    const char * _HEIGHT_ = "h";
    const char * _COLOR_ = "color";
    const char * _LETTER_SPACING_ = "letter-spacing";
    const char * _LINE_OFFSET_ = "line-offset";

    const rgbmatrix::Font *ParseFont(const char * str) {
        return NULL; // BUGBUG todo
    }

    int ParseInt(const char * str, int defaultValue = 0) {
        return atoi(str);
    }

    rgbmatrix::Color ParseColor(const char * str, rgbmatrix::Color defaultValue = rgbmatrix::Color(255, 255, 255)) {
        rgbmatrix::Color c = defaultValue;
        // BUGBUG todo
        return c;
    }

}

namespace wpp = watchpanel; 

wpp::DataManager::DataManager() {
}

wpp::DataManager::~DataManager() {
}

const char * wpp::DataManager::VariableFormatText(const char * formatStr) {
    auto args = fmt::dynamic_format_arg_store<fmt::format_context>();
    args.push_back(fmt::arg("hh", "10"));
    args.push_back(fmt::arg("MM", "11"));
    args.push_back(fmt::arg("city", "Seattle"));
    args.push_back(fmt::arg("icon", "iii"));
    args.push_back(fmt::arg("temp", "It's cold"));
    args.push_back(fmt::arg("temp_hi", "30"));
    args.push_back(fmt::arg("temp_low", "18"));
    std::cerr << formatStr << std::endl;
    fmt::vformat_to_n(std::back_inserter(buffer), 255, formatStr, args);
    std::cerr << buffer.data() << std::endl;
    return buffer.data();
}

wpp::WatchPage::WatchPage() {}

int wpp::WatchPage::LoadPage(const char *path)
{
    Clear();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path);
    if (!result)
    {
        errors.push_back(result.description());
        return -1;
    }

    pugi::xml_node page = doc.child(wpp::_PAGE_);

    pugi::xml_node data = page.child(wpp::_DATA_);
    for (pugi::xml_node data_item = data.first_child(); data_item; data_item = data_item.next_sibling())
    {
        // TODO: data bindings
    }

    pugi::xml_node display = page.child(wpp::_DISPLAY_);
    for (pugi::xml_node graphic_item = display.first_child(); graphic_item; graphic_item = graphic_item.next_sibling())
    {
        wpp::Graphic * graphic = NULL;
        const char * name = graphic_item.name();
        if (strcmp(name, wpp::_TEXT_) == 0) {
            // TEXT graphic
            const char * textPtr = data_manager.VariableFormatText(graphic_item.child_value());
            const rgbmatrix::Font * font = ParseFont(graphic_item.attribute(wpp::_FONT_).value());
            rgbmatrix::Color color = ParseColor(graphic_item.attribute(wpp::_COLOR_).value());
            int x = ParseInt(graphic_item.attribute(wpp::_X_).value());
            int y = ParseInt(graphic_item.attribute(wpp::_Y_).value());
            int letter_spacing = ParseInt(graphic_item.attribute(wpp::_LETTER_SPACING_).value(), 1);
            int line_offset = ParseInt(graphic_item.attribute(wpp::_LINE_OFFSET_).value(), 0);

            graphic = new wpp::TextGraphic(textPtr, font, color, x, y, letter_spacing, line_offset);

        } else if (strcmp(name, wpp::_IMAGE_) == 0) {
            // IMAGE graphic
            graphic = new wpp::ImageGraphic();

        } else if (strcmp(name, wpp::_RECT_) == 0) {
            // RECT graphic
            graphic = new wpp::RectGraphic();

        } else {
            // UNKNOWN graphic
            std::string error("unknown element ");
            error.append(graphic_item.name());
            errors.push_back(error);

            continue;

        }
        display_list.push_back(graphic);
    }
    return 0;
}

void wpp::WatchPage::Draw(rgbmatrix::Canvas *canvas)
{
    for (auto g : display_list)
    {
        g->Draw(canvas);
    }
}

void wpp::WatchPage::Clear() {
    for (auto g : display_list)
    {
        delete g;
    }
    display_list.clear();
    errors.clear();
}

wpp::WatchPage::~WatchPage() {
    Clear();
}

wpp::Graphic::~Graphic() {}

wpp::TextGraphic::TextGraphic(
    const char * textPtr,
    const rgbmatrix::Font * font,
    rgbmatrix::Color color,
    int x,
    int y,
    int letter_spacing,
    int line_offset
) : textPtr(textPtr),
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
     <<  *textPtr << ", " << letter_spacing << ");" << std::endl;
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

wpp::RectGraphic::~RectGraphic() {
}
