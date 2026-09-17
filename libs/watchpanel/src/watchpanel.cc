#include "watchpanel.h"
#include "pugixml.hpp"
#include "timedata.h"
#include "iso_duration.h"

#include <stdlib.h>
#include <iostream>
#include <ctime>

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
    const char * _WRAP_ = "wrap";
    const char * _OVERFLOW_ = "overflow";
    const char * _FEED_ = "feed";
    const char * _NAME_ = "name";
    const char * _HREF_ = "href";
    const char * _TTL_ = "ttl";

    int ParseInt(const char * str, int defaultValue = 0) {
        return atoi(str);
    }

    // Parses a ttl="PT15M"-style ISO-8601 duration attribute, falling back
    // to defaultSeconds if the attribute is absent or malformed.
    long ParseTtlSeconds(const char * iso, long defaultSeconds) {
        long seconds = 0;
        if (iso != NULL && iso[0] != 0 && ParseIsoDuration(iso, &seconds)) {
            return seconds;
        }
        return defaultSeconds;
    }

}

namespace wpp = watchpanel; 

wpp::WatchPage::WatchPage(GraphicsContext *context,
                          const std::string &configPath,
                          const std::string &secretsPath,
                          const std::string &cacheDir)
    : context(context), configPath(configPath), secretsPath(secretsPath), cacheDir(cacheDir) {}

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
    auto configData = new ConfigData("config", configPath.c_str());
    dataList.push_back(configData);

    auto secretsData = new JsonFileData("secrets", secretsPath.c_str());
    dataList.push_back(secretsData);

    // data includes
    pugi::xml_node data = page.child(_DATA_);
    for (pugi::xml_node data_item = data.first_child(); data_item; data_item = data_item.next_sibling())
    {
        DataImport * import = NULL;
        const char * name = data_item.name();
        if (strcmp(name, _FEED_) == 0) {
            const char *feedName = data_item.attribute(_NAME_).value();
            const char *href = data_item.attribute(_HREF_).value();
            const char *ttl = data_item.attribute(_TTL_).value();
            long maxAgeSeconds = ParseTtlSeconds(ttl, 15 * 60);
            import = new FeedData(feedName, href, maxAgeSeconds, cacheDir.c_str());
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
            int text_width = ParseInt(graphic_item.attribute(_WIDTH_).value());
            int text_height = ParseInt(graphic_item.attribute(_HEIGHT_).value());
            int letter_spacing = ParseInt(graphic_item.attribute(_LETTER_SPACING_).value(), 1);
            int line_offset = ParseInt(graphic_item.attribute(_LINE_OFFSET_).value(), 0);
            const char * wrapName = graphic_item.attribute(_WRAP_).value();
            Wrap wrap = (strcmp(wrapName, "word") == 0) ? Wrap::kWord : Wrap::kNone;
            const char * overflowName = graphic_item.attribute(_OVERFLOW_).value();
            Overflow overflow = (strcmp(overflowName, "clip") == 0) ? Overflow::kClip : Overflow::kVisible;

            graphic = new TextGraphic(context, fontName, color, x, y, text_width, text_height,
                                       letter_spacing, line_offset, wrap, overflow);

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
            const char * ttl = graphic_item.attribute(_TTL_).value();
            long imageMaxAgeSeconds = ParseTtlSeconds(ttl, 24 * 60 * 60);
            graphic = new ImageGraphic(context, x, y, width, height, href, imageMaxAgeSeconds);
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

            graphic = new RectGraphic(context, x, y, width, height, fill, stroke);

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

wpp::WatchPanel::WatchPanel(GraphicsContext *context,
                            const std::string &configPath,
                            const std::string &secretsPath,
                            const std::string &cacheDir)
    : context(context),
      configPath(configPath),
      secretsPath(secretsPath),
      cacheDir(cacheDir),
      currentPage(0),
      lastUpdate(0),
      lastPageFlip(0),
      pageInterval(10),
      updateInterval(60) {
}

wpp::WatchPanel::~WatchPanel() {
    Clear();
}

int wpp::WatchPanel::Load(const char *path) {
    auto page = new WatchPage(context, configPath, secretsPath, cacheDir);
    if (page->Load(path) != 0) {
        delete page;
        return -1;
    }
    pageList.push_back(page);
    return 0;
}

void wpp::WatchPanel::Clear() {
    for (auto p : pageList) {
        delete p;
    }
    pageList.clear();
    currentPage = 0;
    lastUpdate = 0;
    lastPageFlip = 0;
}

void wpp::WatchPanel::Update() {
    if (pageList.empty()) {
        return;
    }

    const long now = static_cast<long>(std::time(nullptr));
    if (lastUpdate == 0 || now - lastUpdate >= updateInterval) {
        pageList[currentPage]->Update();
        lastUpdate = now;
    }

    if (pageList.size() > 1 && (lastPageFlip == 0 || now - lastPageFlip >= pageInterval)) {
        currentPage = (currentPage + 1) % static_cast<int>(pageList.size());
        pageList[currentPage]->Update();
        lastPageFlip = now;
    }
}

void wpp::WatchPanel::Draw() {
    if (pageList.empty()) {
        return;
    }
    pageList[currentPage]->Draw();
}

