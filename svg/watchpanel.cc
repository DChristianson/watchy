#include "watchpanel.h"
#include "pugixml.hpp"
#include "timedata.h"

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
    const char * _FILL_ = "fill";
    const char * _STROKE_ = "stroke";
    const char * _WIDTH_ = "width";
    const char * _HEIGHT_ = "height";
    const char * _COLOR_ = "color";
    const char * _LETTER_SPACING_ = "letter-spacing";
    const char * _LINE_OFFSET_ = "line-offset";
    const char * _FEED_ = "feed";
    const char * _NAME_ = "name";
    const char * _HREF_ = "href";

    int ParseInt(const char * str, int defaultValue = 0) {
        return atoi(str);
    }

}

namespace wpp = watchpanel; 

wpp::WatchPage::WatchPage(Canvas *canvas) : canvas(canvas) {}

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

    pugi::xml_node page = doc.child(_PAGE_);

    // builtin data bindings
    auto timeData = new TimeData();
    dataList.push_back(timeData);
    auto configData = new ConfigData("config", "config.json");
    dataList.push_back(configData);

    // data includes
    pugi::xml_node data = page.child(_DATA_);
    for (pugi::xml_node data_item = data.first_child(); data_item; data_item = data_item.next_sibling())
    {
        DataImport * import = NULL;
        const char * name = data_item.name();
        if (strcmp(name, _FEED_) == 0) {
            const char *feedName = data_item.attribute(_NAME_).value();
            const char *href = data_item.attribute(_HREF_).value();
            import = new FeedData(feedName, href);
            if (FormattedString::IsTemplatized(href)) {
                import->AddUpdate(
                    new UpdateFormattedString(
                        href, 
                        [import] (const char* v) { ((FeedData *)import)->SetHRef(v); }
                    )
                );
            }
        }
        if (NULL != import) {
            dataList.push_back(import);
        }
    }

    // display list
    pugi::xml_node display = page.child(_DISPLAY_);
    for (pugi::xml_node graphic_item = display.first_child(); graphic_item; graphic_item = graphic_item.next_sibling())
    {
        Graphic * graphic = NULL;
        const char * name = graphic_item.name();
        if (strcmp(name, _TEXT_) == 0) {
            // TEXT graphic
            const char * fontName = graphic_item.attribute(_FONT_).value();
            const char * colorName = graphic_item.attribute(_COLOR_).value();
            Color color = Color::Parse(colorName);
            int x = ParseInt(graphic_item.attribute(_X_).value());
            int y = ParseInt(graphic_item.attribute(_Y_).value());
            int letter_spacing = ParseInt(graphic_item.attribute(_LETTER_SPACING_).value(), 1);
            int line_offset = ParseInt(graphic_item.attribute(_LINE_OFFSET_).value(), 0);

            graphic = new TextGraphic(canvas, fontName, color, x, y, letter_spacing, line_offset);

            pugi::xml_node span = graphic_item.child("tspan");
            if (span) {
                while (span) {
                    const char * text = span.child_value();
                    LoadText(text, (TextGraphic *)graphic);
                    span = span.next_sibling("tspan");
                }
            } else {
                const char * text = graphic_item.child_value();
                LoadText(text, (TextGraphic *)graphic);

            }

        } else if (strcmp(name, _IMAGE_) == 0) {
            // IMAGE graphic
            int x = ParseInt(graphic_item.attribute(_X_).value());
            int y = ParseInt(graphic_item.attribute(_Y_).value());
            int width = ParseInt(graphic_item.attribute(_WIDTH_).value());
            int height = ParseInt(graphic_item.attribute(_HEIGHT_).value());
            const char * href = graphic_item.attribute(_HREF_).value();
            graphic = new ImageGraphic(canvas, x, y, width, height, href);
            if (FormattedString::IsTemplatized(href)) {
                updateList.push_back(
                    new UpdateFormattedString(
                        href, 
                        [graphic] (const char* v) { ((ImageGraphic *)graphic)->SetHRef(v); }
                    )
                );
            }

        } else if (strcmp(name, _RECT_) == 0) {
            // RECT graphic
            int x = ParseInt(graphic_item.attribute(_X_).value());
            int y = ParseInt(graphic_item.attribute(_Y_).value());
            int width = ParseInt(graphic_item.attribute(_WIDTH_).value());
            int height = ParseInt(graphic_item.attribute(_HEIGHT_).value());
            const char * fillName = graphic_item.attribute(_FILL_).value();
            Color fill = Color::Parse(fillName);
            const char * strokeName = graphic_item.attribute(_STROKE_).value();
            Color stroke = Color::Parse(strokeName);

            graphic = new RectGraphic(canvas, x, y, width, height, fill, stroke);

        } else {
            // UNKNOWN graphic
            std::string error("unknown element ");
            error.append(graphic_item.name());
            errors.push_back(error);

            continue;

        }
        if (NULL != graphic) {
            displayList.push_back(graphic);
        }
    }
    return 0;
}

void wpp::WatchPage::LoadText(const char *text, TextGraphic *textGraphic) {
    TextSpan *span = &(textGraphic->AppendText(text));

    if (FormattedString::IsTemplatized(text)) {
        updateList.push_back(new UpdateFormattedString(text, [span] (const char* v) { span->text = v; }));
    }

}

void wpp::WatchPage::Update() {

    rapidjson::Document root(rapidjson::kObjectType);
    auto model = DocumentModel(root);
    for (auto d : dataList) {
        d->Update(model);
        rapidjson::Document out(rapidjson::kObjectType);
        d->Pull(model, out);
        rapidjson::Value subtree(rapidjson::kObjectType);
        subtree.CopyFrom(out, root.GetAllocator());
        root.AddMember(rapidjson::StringRef(d->GetName()), subtree, root.GetAllocator());
    }

    for (auto u : updateList) {
        u->Update(model);
    }

}

void wpp::WatchPage::Draw()
{
    for (auto g : displayList)
    {
        std::cout << "drawing graphic" << std::endl;
        g->Draw();
    }
}

void wpp::WatchPage::Clear() {
    for (auto d : dataList)
    {
        delete d;
    }
    dataList.clear();
    
    for (auto u : updateList)
    {
        delete u;
    }
    updateList.clear();

    for (auto g : displayList)
    {
        delete g;
    }
    displayList.clear();
    
    errors.clear();
}

wpp::WatchPage::~WatchPage() {
    Clear();
}

