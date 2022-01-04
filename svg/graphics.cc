#include "graphics.h"
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

void wpp::Color::Format(std::string &out) {
    out = "#FFFFFF"; 
}

wpp::Color wpp::Color::Parse(const char *colorName) {
    if (NULL == colorName) {
        return ColorBlack;        
    }
    const char firstChar = colorName[0];
    if (0 == firstChar) {
        return ColorBlack;
    } else if ('#' == firstChar) {
        // Three digit hex — #rgb
        // Six digit hex — #rrggbb
        // TODO: parse
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

wpp::Canvas::~Canvas() {}

void wpp::Canvas::DrawText(
    const TextSpan *textSpan,
    const char *fontName,
    Color color,
    int x,
    int y,
    int letterSpacing,
    int lineOffset
) {}

void wpp::Canvas::DrawImage(
    int x,
    int y,
    int width,
    int height,
    const char *href
) {}

void wpp::Canvas::DrawRect(
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke
) {}

wpp::Graphic::~Graphic() {}

wpp::TextGraphic::TextGraphic(
    Canvas * canvas,
    const char *fontName,
    wpp::Color color,
    int x,
    int y,
    int letterSpacing,
    int lineOffset
) : Graphic(canvas),
    firstSpan(0),
    lastSpan(0),
    fontName(fontName), 
    color(color), 
    x(x), 
    y(y), 
    letterSpacing(letterSpacing), 
    lineOffset(lineOffset)
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
    canvas->DrawText(firstSpan, fontName.c_str(), color, x, y, letterSpacing, lineOffset);
}

wpp::TextGraphic::~TextGraphic() {
    TextSpan *currentSpan = firstSpan;
    while (currentSpan) {
        TextSpan *nextSpan = currentSpan->nextSpan;
        delete currentSpan;
        currentSpan = nextSpan;
    }
}

wpp::ImageGraphic::ImageGraphic(
    wpp::Canvas * canvas,
    int x,
    int y,
    int width,
    int height,
    const char *href
) : Graphic(canvas),
    x(x),
    y(y),
    width(width),
    height(height),
    href(href) {}

void wpp::ImageGraphic::SetHRef(const char *value) {
    href = value;
}

void wpp::ImageGraphic::Draw()
{
    canvas->DrawImage(x, y, width, height, href.c_str());
}

wpp::ImageGraphic::~ImageGraphic() {}

wpp::RectGraphic::RectGraphic(
    wpp::Canvas *canvas,
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke
) : Graphic(canvas),
    x(x),
    y(y),
    width(width),
    height(height),
    fill(fill),
    stroke(stroke) {}

void wpp::RectGraphic::Draw()
{
    canvas->DrawRect(x, y, width, height, fill, stroke);
}

wpp::RectGraphic::~RectGraphic() {}
