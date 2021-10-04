#include "watchpanel.h"
#include "pugixml.hpp"

#include <stdlib.h>
#include <iostream>

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
    const char * _INCLUDE_ = "include";
    const char * _SRC_ = "src";

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

wpp::WatchPage::WatchPage() {}

int wpp::WatchPage::Load(const char *path)
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

    // builtin data bindings
    auto builtin = new BuiltinData();
    data_list.push_back(builtin);

    // data includes
    pugi::xml_node data = page.child(wpp::_DATA_);
    for (pugi::xml_node data_item = data.first_child(); data_item; data_item = data_item.next_sibling())
    {
        wpp::DataImport * import = NULL;
        const char * name = data_item.name();
        std::cout << name << std::endl;
        if (strcmp(name, wpp::_INCLUDE_) == 0) {
            const char * src = data_item.attribute(wpp::_SRC_).value();
            import = new wpp::RemoteFetchData(src);
        }
        if (NULL != import) {
            data_list.push_back(import);
        }
    }

    // display list
    pugi::xml_node display = page.child(wpp::_DISPLAY_);
    for (pugi::xml_node graphic_item = display.first_child(); graphic_item; graphic_item = graphic_item.next_sibling())
    {
        wpp::Graphic * graphic = NULL;
        const char * name = graphic_item.name();
        if (strcmp(name, wpp::_TEXT_) == 0) {
            // TEXT graphic
            const char * text = graphic_item.child_value();
            const rgbmatrix::Font * font = ParseFont(graphic_item.attribute(wpp::_FONT_).value());
            rgbmatrix::Color color = ParseColor(graphic_item.attribute(wpp::_COLOR_).value());
            int x = ParseInt(graphic_item.attribute(wpp::_X_).value());
            int y = ParseInt(graphic_item.attribute(wpp::_Y_).value());
            int letter_spacing = ParseInt(graphic_item.attribute(wpp::_LETTER_SPACING_).value(), 1);
            int line_offset = ParseInt(graphic_item.attribute(wpp::_LINE_OFFSET_).value(), 0);

            const bool is_dynamic_text = wpp::FormattedString::IsTemplatized(text);
            const char *value;
            if (is_dynamic_text) {
                value = "";
            } else {
                value = text;
            }

            graphic = new wpp::TextGraphic(text, font, color, x, y, letter_spacing, line_offset);

            if (is_dynamic_text) {
                update_list.push_back( new wpp::UpdateText( text, *(TextGraphic *)graphic ) );
            }

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
        if (NULL != graphic) {
            display_list.push_back(graphic);
        }
    }
    return 0;
}

void wpp::WatchPage::Update() {

    std::map<std::string, std::string> vars;

    for (auto d : data_list) {
        std::cout << "pulling data" << std::endl;
        d->Pull(vars);
    }

    for (auto u : update_list) {
        std::cout << "updating graphic" << std::endl;
        u->Update(vars);
    }

}

void wpp::WatchPage::Draw(rgbmatrix::Canvas *canvas)
{
    for (auto g : display_list)
    {
        g->Draw(canvas);
    }
}

void wpp::WatchPage::Clear() {
    for (auto d : data_list)
    {
        delete d;
    }
    data_list.clear();
    
    for (auto u : update_list)
    {
        delete u;
    }
    update_list.clear();

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

