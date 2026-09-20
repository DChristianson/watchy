#include "graphics.h"
#include "graphics_context.h"
#include "model.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <iostream>

namespace wpp = watchpanel; 

// Svg Tiny colors
const wpp::Color ColorBlack(0, 0, 0);
const wpp::Color ColorGreen(0, 128, 0);
const wpp::Color ColorSilver(192, 192, 192);
const wpp::Color ColorLime(0, 255, 0);
const wpp::Color ColorGray(128, 128, 128);
const wpp::Color ColorOlive(128, 128, 0);
const wpp::Color ColorWhite(255, 255, 255);
const wpp::Color ColorYellow(255, 255, 0);
const wpp::Color ColorMaroon(128, 0, 0);
const wpp::Color ColorNavy(0, 0, 128);
const wpp::Color ColorRed(255, 0, 0);
const wpp::Color ColorBlue(0, 0, 255);
const wpp::Color ColorPurple(128, 0, 128);
const wpp::Color ColorTeal(0, 128, 128);
const wpp::Color ColorFuchsia(255, 0, 255);
const wpp::Color ColorAqua(0, 255, 255);


std::map<std::string, wpp::Color> ColorKeywords = {
    {"black", ColorBlack},
    {"green", ColorGreen},
    {"silver", ColorSilver},
    {"lime", ColorLime},
    {"gray", ColorGray},
    {"olive", ColorOlive},
    {"white", ColorWhite},
    {"yellow", ColorYellow},
    {"maroon", ColorMaroon},
    {"navy", ColorNavy},
    {"red", ColorRed},
    {"blue", ColorBlue},
    {"purple", ColorPurple},
    {"teal", ColorTeal},
    {"fuchsia", ColorFuchsia},
    {"aqua", ColorAqua},
};

void wpp::Color::Format(std::string &out) const {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
    out = buf;
}

namespace {

int HexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

}  // namespace

wpp::Color wpp::Color::Parse(const char *colorName) {
    if (NULL == colorName) {
        return ColorBlack;
    }
    const char firstChar = colorName[0];
    if (0 == firstChar) {
        return ColorBlack;
    } else if ('#' == firstChar) {
        std::string hex(colorName + 1);
        if (hex.size() == 3) {
            // Three digit hex — #rgb, expand to #rrggbb
            std::string expanded;
            for (char c : hex) {
                expanded += c;
                expanded += c;
            }
            hex = expanded;
        }
        if (hex.size() == 6) {
            const uint8_t r = static_cast<uint8_t>(HexDigit(hex[0]) * 16 + HexDigit(hex[1]));
            const uint8_t g = static_cast<uint8_t>(HexDigit(hex[2]) * 16 + HexDigit(hex[3]));
            const uint8_t b = static_cast<uint8_t>(HexDigit(hex[4]) * 16 + HexDigit(hex[5]));
            return wpp::Color(r, g, b);
        }
    // } else if (starts_with(colorName, "rgb(")) {
        // Integer functional — rgb(rrr, ggg, bbb)
        // Float functional — rgb(R%, G%, B%)
        // TODO: parse
    } else {
        // Color keyword
        auto it = ColorKeywords.find(colorName);
        if (it != ColorKeywords.end()) {
            return it->second;
        }
    }
    return ColorBlack;        
}

wpp::Graphic::~Graphic() {}

wpp::TextGraphic::TextGraphic(
    GraphicsContext * context,
    const char *fontName,
    wpp::Color color,
    int x,
    int y,
    int width,
    int height,
    int letterSpacing,
    int lineOffset,
    Wrap wrap,
    Overflow overflow
) : Graphic(context),
    firstSpan(0),
    lastSpan(0),
    fontName(fontName),
    color(color),
    x(x),
    y(y),
    width(width),
    height(height),
    letterSpacing(letterSpacing),
    lineOffset(lineOffset),
    wrap(wrap),
    overflow(overflow),
    scrollOffsetY(0),
    lastContentHeight(0)
{}

wpp::TextSpan &wpp::TextGraphic::AppendText(const char *text) {
    TextSpan *nextSpan = new TextSpan(text);
    if (firstSpan) {
        lastSpan->nextSpan = nextSpan;
        lastSpan = nextSpan;
    } else {
        firstSpan = nextSpan;
        lastSpan = nextSpan;
    }
    return *nextSpan;
}

void wpp::TextGraphic::Draw()
{
    lastContentHeight = context->DrawText(
        firstSpan, fontName.c_str(), color, x, y, width, height,
        letterSpacing, lineOffset, wrap, overflow, scrollOffsetY);
}

wpp::TextGraphic::~TextGraphic() {
    TextSpan *currentSpan = firstSpan;
    while (currentSpan) {
        TextSpan *nextSpan = currentSpan->nextSpan;
        delete currentSpan;
        currentSpan = nextSpan;
    }
}

wpp::GroupGraphic::GroupGraphic(GraphicsContext *context) : Graphic(context) {}

wpp::GroupGraphic::~GroupGraphic() {
    for (auto c : children) {
        delete c;
    }
}

void wpp::GroupGraphic::AddChild(Graphic *child) {
    children.push_back(child);
}

void wpp::GroupGraphic::Draw() {
    for (auto c : children) {
        c->Draw();
    }
}

wpp::ImageGraphic::ImageGraphic(
    GraphicsContext * context,
    int x,
    int y,
    int width,
    int height,
    const char *href,
    long maxAgeSeconds
) : Graphic(context),
    x(x),
    y(y),
    width(width),
    height(height),
    href(href),
    maxAgeSeconds(maxAgeSeconds) {}

void wpp::ImageGraphic::SetHRef(const char *value) {
    href = value;
}

void wpp::ImageGraphic::Draw()
{
    context->DrawImage(x, y, width, height, href.c_str(), maxAgeSeconds);
}

wpp::ImageGraphic::~ImageGraphic() {}

wpp::RectGraphic::RectGraphic(
    GraphicsContext *context,
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke
) : Graphic(context),
    x(x),
    y(y),
    width(width),
    height(height),
    fill(fill),
    stroke(stroke) {}

void wpp::RectGraphic::Draw()
{
    context->DrawRect(x, y, width, height, fill, stroke);
}

wpp::RectGraphic::~RectGraphic() {}

wpp::FlipGraphic::FlipGraphic(
    GraphicsContext * context,
    const char *fontName,
    Color color,
    int x,
    int y,
    int width,
    int height,
    int letterSpacing,
    int lineOffset,
    Wrap wrap,
    Overflow overflow,
    const char *itemsPath,
    long periodSeconds,
    int scrollSpeedPxPerSec
) : Graphic(context),
    inner(new TextGraphic(context, fontName, color, x, y, width, height, letterSpacing, lineOffset, wrap, overflow)),
    itemsPath(itemsPath),
    periodSeconds(periodSeconds),
    currentIndex(0),
    lastFlipTime(0),
    hasFlippedOnce(false),
    height(height),
    scrollSpeedPxPerSec(scrollSpeedPxPerSec),
    scrollOffsetPx(0)
{}

wpp::FlipGraphic::~FlipGraphic() {
    delete inner;
    for (auto u : childUpdates) {
        delete u;
    }
}

wpp::TextSpan &wpp::FlipGraphic::AppendText(const char *text) {
    return inner->AppendText(text);
}

void wpp::FlipGraphic::AddChildUpdate(Updateable *update) {
    childUpdates.push_back(update);
}

void wpp::FlipGraphic::Update(const Model &model, long now, long deltaSeconds) {
    const int itemCount = model.GetArraySize(itemsPath.c_str());

    if (itemCount <= 0) {
        currentIndex = 0;
        return;
    }
    if (currentIndex >= itemCount) {
        currentIndex = 0;  // items array shrank out from under us
    }

    bool advanced = false;
    if (!hasFlippedOnce) {
        // Show the first item immediately rather than waiting a full
        // period before anything appears.
        lastFlipTime = now;
        hasFlippedOnce = true;
        advanced = true;
    } else if (now - lastFlipTime >= periodSeconds) {
        currentIndex = (currentIndex + 1) % itemCount;
        lastFlipTime = now;
        advanced = true;
    }

    // Reset the scroll to the top on every new item so each one gets read
    // from the start; otherwise, if this item's content exceeds the box,
    // creep it upward at scrollSpeedPxPerSec and stop once fully revealed
    // (no looping) -- matches how much of it was actually visible on the
    // most recent Draw() (see TextGraphic::LastContentHeight).
    if (advanced) {
        scrollOffsetPx = 0;
    } else if (scrollSpeedPxPerSec > 0) {
        const int maxScroll = std::max(0, inner->LastContentHeight() - height);
        scrollOffsetPx = std::min(maxScroll, scrollOffsetPx + static_cast<int>(scrollSpeedPxPerSec * deltaSeconds));
    }
    inner->SetScrollOffset(scrollOffsetPx);

    const std::string scopedBase = itemsPath + "/" + std::to_string(currentIndex);
    ScopedModel scoped(model, scopedBase);
    for (auto u : childUpdates) {
        u->Update(scoped, now, deltaSeconds);
    }
}

void wpp::FlipGraphic::Draw() {
    inner->Draw();
}
