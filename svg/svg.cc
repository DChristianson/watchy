#include "svg.h"
    
#include <iostream>

namespace wpp = watchpanel; 

wpp::SvgCanvas::SvgCanvas(int width, int height) {
    root = doc.append_child("svg");
    root.append_attribute("xmlns").set_value("http://www.w3.org/2000/svg");
    root.append_attribute("width").set_value(width);
    root.append_attribute("height").set_value(height);
}

wpp::SvgCanvas::~SvgCanvas() {}

void wpp::SvgCanvas::DrawText(
    const TextSpan *textSpan,
    const char *fontName,
    wpp::Color color,
    int x,
    int y,
    int letterSpacing,
    int lineOffset)
{
    pugi::xml_node text = root.append_child("text");
    text.append_attribute("x").set_value(x);
    text.append_attribute("y").set_value(y);
    // TODO: fonts
    // TODO: color
    // TODO: line offset compute
    const char *lineOffsetEms = "1em";
    while (textSpan) {
        pugi::xml_node tspan = text.append_child("tspan");
        tspan.append_attribute("x").set_value(0);
        tspan.append_attribute("dy").set_value(lineOffsetEms);
        tspan.append_child(pugi::node_pcdata).set_value(textSpan->text.c_str());
        textSpan = textSpan->nextSpan;
    }
}

void wpp::SvgCanvas::DrawImage(
    int x,
    int y,
    int width,
    int height,
    const char *href)
{
    pugi::xml_node node = root.append_child("image");
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
    pugi::xml_node node = root.append_child("rect");
    node.append_attribute("x").set_value(x);
    node.append_attribute("y").set_value(y);
    node.append_attribute("width").set_value(width);
    node.append_attribute("height").set_value(height);
    //TODO: represent color
    //node.append_attribute("fill").set_value(fill);
    //node.append_attribute("stroke").set_value(stroke);
}

void wpp::SvgCanvas::Save(const char *path) {
    doc.save_file(path);
}
