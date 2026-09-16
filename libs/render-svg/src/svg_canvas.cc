#include "svg_canvas.h"
#include "watchpanel/bdf_font.h"

#include <iostream>

namespace wpp = watchpanel;

wpp::SvgCanvas::SvgCanvas(int width, int height, int pixelScale, const std::string &fontPath)
    : width(width), height(height), pixelScale(pixelScale), fontPath(fontPath) {
    Reset();
}

wpp::SvgCanvas::~SvgCanvas() {}

void wpp::SvgCanvas::Reset() {
    root = doc.append_child("svg");
    root.append_attribute("xmlns").set_value("http://www.w3.org/2000/svg");
    root.append_attribute("width").set_value(width * pixelScale);
    root.append_attribute("height").set_value(height * pixelScale);
    root.append_attribute("shape-rendering").set_value("crispEdges");

    // Everything is drawn in native canvas-pixel coordinates into this
    // group, then scaled up as a whole so the frame renders as chunky,
    // crisp pixels instead of tiny native-resolution squares.
    scene = root.append_child("g");
    std::string transform = "scale(" + std::to_string(pixelScale) + ")";
    scene.append_attribute("transform").set_value(transform.c_str());
}

void wpp::SvgCanvas::Clear() {
    doc.reset();
    Reset();
}

void wpp::SvgCanvas::DrawGlyph(unsigned char ch, const Color &color, int x, int y) {
    if (fontPath.empty()) return;
    const BdfFont &font = BdfFont::Load(fontPath);
    const BdfGlyph *glyph = font.Find(static_cast<int>(ch));
    if (!glyph) return;

    std::string colorHex;
    color.Format(colorHex);

    for (int gy = 0; gy < glyph->height; ++gy) {
        for (int gx = 0; gx < glyph->width; ++gx) {
            if (!glyph->GetBit(gx, gy)) continue;
            pugi::xml_node rect = scene.append_child("rect");
            rect.append_attribute("x").set_value(x + gx);
            rect.append_attribute("y").set_value(y + gy);
            rect.append_attribute("width").set_value(1);
            rect.append_attribute("height").set_value(1);
            rect.append_attribute("fill").set_value(colorHex.c_str());
        }
    }
}

void wpp::SvgCanvas::DrawText(
    const TextSpan *textSpan,
    const char *fontName,
    wpp::Color color,
    int x,
    int y,
    int letterSpacing,
    int lineOffset)
{
    (void)fontName;
    (void)lineOffset;
    int cursorX = x;
    while (textSpan) {
        const std::string &s = textSpan->text;
        for (size_t i = 0; i < s.size(); ++i) {
            const unsigned char ch = static_cast<unsigned char>(s[i]);
            DrawGlyph(ch, color, cursorX, y);
            cursorX += 4 + letterSpacing;
        }
        textSpan = textSpan->nextSpan;
        if (textSpan) {
            cursorX += 2;
        }
    }
}

void wpp::SvgCanvas::DrawImage(
    int x,
    int y,
    int width,
    int height,
    const char *href)
{
    pugi::xml_node node = scene.append_child("image");
    node.append_attribute("x").set_value(x);
    node.append_attribute("y").set_value(y);
    node.append_attribute("width").set_value(width);
    node.append_attribute("height").set_value(height);
    node.append_attribute("href").set_value(href);
}

void wpp::SvgCanvas::DrawRect(
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke
) {
    pugi::xml_node node = scene.append_child("rect");
    node.append_attribute("x").set_value(x);
    node.append_attribute("y").set_value(y);
    node.append_attribute("width").set_value(width);
    node.append_attribute("height").set_value(height);

    std::string fillHex;
    fill.Format(fillHex);
    node.append_attribute("fill").set_value(fillHex.c_str());

    std::string strokeHex;
    stroke.Format(strokeHex);
    node.append_attribute("stroke").set_value(strokeHex.c_str());
}

void wpp::SvgCanvas::Save(const char *path) {
    doc.save_file(path);
}
