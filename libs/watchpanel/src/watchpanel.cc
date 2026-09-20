#include "watchpanel.h"
#include "pugixml.hpp"
#include "timedata.h"
#include "iso_duration.h"

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
    const char * _WRAP_ = "wrap";
    const char * _OVERFLOW_ = "overflow";
    const char * _FEED_ = "feed";
    const char * _NAME_ = "name";
    const char * _HREF_ = "href";
    const char * _TTL_ = "ttl";
    const char * _FORMAT_ = "format";
    const char * _FLIP_ = "flip";
    const char * _PATH_ = "path";
    const char * _PERIOD_ = "period";
    const char * _SCROLL_SPEED_ = "scroll-speed";
    const char * _ATTRIBUTION_ = "attribution";
    const char * _HOLD_ = "hold";
    const char * _FADE_ = "fade";

    int ParseInt(const char * str, int defaultValue = 0) {
        return atoi(str);
    }

    // Parses a ttl="PT15M"/period="PT5S"-style ISO-8601 duration attribute,
    // falling back to defaultSeconds if the attribute is absent or malformed.
    long ParseDurationSeconds(const char * iso, long defaultSeconds) {
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
    : pageTransition(nullptr), context(context), configPath(configPath), secretsPath(secretsPath), cacheDir(cacheDir) {}

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
            long maxAgeSeconds = ParseDurationSeconds(ttl, 15 * 60);
            const char *format = data_item.attribute(_FORMAT_).value();
            import = new FeedData(feedName, href, maxAgeSeconds, cacheDir.c_str(),
                                   (format != NULL && format[0] != 0) ? format : "json");
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

    // Optional <attribution>: shown on load, then true-cross-fades into the
    // rest of the display list. When present, every display-list graphic
    // below is built against pageTransition->ToContext() (an off-screen
    // buffer) instead of the real context directly, and gets wrapped in a
    // GroupGraphic as the transition's "to" side once the loop is done.
    GraphicsContext * drawContext = context;
    pugi::xml_node attribution = page.child(_ATTRIBUTION_);
    if (attribution) {
        long holdSeconds = ParseDurationSeconds(attribution.attribute(_HOLD_).value(), 3);
        long fadeSeconds = ParseDurationSeconds(attribution.attribute(_FADE_).value(), 2);

        pageTransition = new FadeTransitionGraphic(context, 0, 0, context->Width(), context->Height(),
                                                    holdSeconds, fadeSeconds, context->FontPath(), cacheDir);
        drawContext = pageTransition->ToContext();

        const char * fontName = attribution.attribute(_FONT_).value();
        const char * colorName = attribution.attribute(_COLOR_).value();
        Color color = Color::Parse(colorName);
        int x = ParseInt(attribution.attribute(_X_).value());
        int y = ParseInt(attribution.attribute(_Y_).value());
        int width = ParseInt(attribution.attribute(_WIDTH_).value());
        int height = ParseInt(attribution.attribute(_HEIGHT_).value());
        int letter_spacing = ParseInt(attribution.attribute(_LETTER_SPACING_).value(), 1);
        int line_offset = ParseInt(attribution.attribute(_LINE_OFFSET_).value(), 0);
        const char * wrapName = attribution.attribute(_WRAP_).value();
        Wrap wrap = (strcmp(wrapName, "word") == 0) ? Wrap::kWord : Wrap::kNone;
        const char * overflowName = attribution.attribute(_OVERFLOW_).value();
        Overflow overflow = (strcmp(overflowName, "clip") == 0) ? Overflow::kClip : Overflow::kVisible;

        TextGraphic * attributionText = new TextGraphic(pageTransition->FromContext(), fontName, color,
                                                          x, y, width, height, letter_spacing, line_offset,
                                                          wrap, overflow);
        pugi::xml_node span = attribution.child("tspan");
        if (span) {
            while (span) {
                const char * text = span.child_value();
                LoadText(text, attributionText);
                span = span.next_sibling("tspan");
            }
        } else {
            const char * text = attribution.child_value();
            LoadText(text, attributionText);
        }
        pageTransition->SetFromGraphic(attributionText);
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

            graphic = new TextGraphic(drawContext, fontName, color, x, y, text_width, text_height,
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
            long imageMaxAgeSeconds = ParseDurationSeconds(ttl, 24 * 60 * 60);
            graphic = new ImageGraphic(drawContext, x, y, width, height, href, imageMaxAgeSeconds);
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

            graphic = new RectGraphic(drawContext, x, y, width, height, fill, stroke);

        } else if (strcmp(name, _FLIP_) == 0) {
            // FLIP graphic: cycles through the elements of the array at
            // `path`, re-scoping its children's relative template paths to
            // whichever item is currently showing.
            const char * fontName = graphic_item.attribute(_FONT_).value();
            const char * colorName = graphic_item.attribute(_COLOR_).value();
            Color color = Color::Parse(colorName);
            int x = ParseInt(graphic_item.attribute(_X_).value());
            int y = ParseInt(graphic_item.attribute(_Y_).value());
            int flip_width = ParseInt(graphic_item.attribute(_WIDTH_).value());
            int flip_height = ParseInt(graphic_item.attribute(_HEIGHT_).value());
            int letter_spacing = ParseInt(graphic_item.attribute(_LETTER_SPACING_).value(), 1);
            int line_offset = ParseInt(graphic_item.attribute(_LINE_OFFSET_).value(), 0);
            const char * wrapName = graphic_item.attribute(_WRAP_).value();
            Wrap wrap = (strcmp(wrapName, "word") == 0) ? Wrap::kWord : Wrap::kNone;
            const char * overflowName = graphic_item.attribute(_OVERFLOW_).value();
            Overflow overflow = (strcmp(overflowName, "clip") == 0) ? Overflow::kClip : Overflow::kVisible;
            const char * itemsPath = graphic_item.attribute(_PATH_).value();
            const char * periodIso = graphic_item.attribute(_PERIOD_).value();
            long periodSeconds = ParseDurationSeconds(periodIso, 5);
            int scrollSpeed = ParseInt(graphic_item.attribute(_SCROLL_SPEED_).value(), 0);

            FlipGraphic * flip = new FlipGraphic(drawContext, fontName, color, x, y, flip_width, flip_height,
                                                  letter_spacing, line_offset, wrap, overflow,
                                                  itemsPath, periodSeconds, scrollSpeed);
            graphic = flip;
            flipUpdates.push_back(flip);

            for (pugi::xml_node span = graphic_item.child("tspan"); span; span = span.next_sibling("tspan")) {
                const char * text = span.child_value();
                TextSpan * textSpan = &(flip->AppendText(text));
                if (FormattedString::IsTemplatized(text)) {
                    flip->AddChildUpdate(new UpdateFormattedString(text, [textSpan] (const char* v) { textSpan->text = v; }));
                }
            }

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

    if (pageTransition) {
        // Ownership of everything in displayList moves to this group (and
        // from there, to pageTransition) -- see Clear().
        GroupGraphic * group = new GroupGraphic(drawContext);
        for (auto g : displayList) {
            group->AddChild(g);
        }
        pageTransition->SetToGraphic(group);
    }

    return 0;
}

void wpp::WatchPage::LoadText(const char *text, TextGraphic *textGraphic) {
    TextSpan *span = &(textGraphic->AppendText(text));

    if (FormattedString::IsTemplatized(text)) {
        updateList.push_back(new UpdateFormattedString(text, [span] (const char* v) { span->text = v; }));
    }

}

void wpp::WatchPage::Update(long now, long deltaSeconds) {

    rapidjson::Document root(rapidjson::kObjectType);
    auto model = DocumentModel(root);
    for (auto d : dataList) {
        d->Update(model, now, deltaSeconds);
        rapidjson::Document out(rapidjson::kObjectType);
        d->Pull(model, out, now, deltaSeconds);
        rapidjson::Value subtree(rapidjson::kObjectType);
        subtree.CopyFrom(out, root.GetAllocator());
        root.AddMember(rapidjson::StringRef(d->GetName()), subtree, root.GetAllocator());
    }

    for (auto u : updateList) {
        u->Update(model, now, deltaSeconds);
    }

    for (auto f : flipUpdates) {
        f->Update(model, now, deltaSeconds);
    }

    // Drives only the attribution fade's own clock -- the display list's
    // own dynamic content is already covered by updateList/flipUpdates
    // above, regardless of which context it was built against.
    if (pageTransition) {
        pageTransition->Update(model, now, deltaSeconds);
    }

}

void wpp::WatchPage::Draw()
{
    if (pageTransition) {
        pageTransition->Draw();
        return;
    }
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

    if (pageTransition) {
        // Owns the attribution graphic and, via its GroupGraphic "to"
        // side, every displayList entry too -- displayList itself is a
        // non-owning alias list in this case (see the field comment).
        delete pageTransition;
        pageTransition = nullptr;
    } else {
        for (auto g : displayList)
        {
            delete g;
        }
    }
    displayList.clear();

    // Non-owning aliases into displayList -- already deleted above.
    flipUpdates.clear();

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

void wpp::WatchPanel::Update(long now, long deltaSeconds) {
    if (pageList.empty()) {
        return;
    }

    if (lastUpdate == 0 || now - lastUpdate >= updateInterval) {
        pageList[currentPage]->Update(now, deltaSeconds);
        lastUpdate = now;
    }

    if (pageList.size() > 1 && (lastPageFlip == 0 || now - lastPageFlip >= pageInterval)) {
        currentPage = (currentPage + 1) % static_cast<int>(pageList.size());
        pageList[currentPage]->Update(now, deltaSeconds);
        lastPageFlip = now;
    }
}

void wpp::WatchPanel::Draw() {
    if (pageList.empty()) {
        return;
    }
    pageList[currentPage]->Draw();
}

