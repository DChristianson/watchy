#include "svg_raster.h"

namespace wpp = watchpanel;

wpp::SvgRaster::SvgRaster(int width, int height, int pixelScale)
    : width_(width), height_(height), pixelScale_(pixelScale) {
    Reset();
}

wpp::SvgRaster::~SvgRaster() {}

void wpp::SvgRaster::Reset() {
    root = doc.append_child("svg");
    root.append_attribute("xmlns").set_value("http://www.w3.org/2000/svg");
    root.append_attribute("width").set_value(width_ * pixelScale_);
    root.append_attribute("height").set_value(height_ * pixelScale_);
    root.append_attribute("shape-rendering").set_value("crispEdges");

    // Everything is drawn in native canvas-pixel coordinates into this
    // group, then scaled up as a whole.
    scene = root.append_child("g");
    std::string transform = "scale(" + std::to_string(pixelScale_) + ")";
    scene.append_attribute("transform").set_value(transform.c_str());
}

void wpp::SvgRaster::Clear() {
    doc.reset();
    Reset();
}

void wpp::SvgRaster::SetPixel(int x, int y, Color color) {
    std::string colorHex;
    color.Format(colorHex);

    pugi::xml_node rect = scene.append_child("rect");
    rect.append_attribute("x").set_value(x);
    rect.append_attribute("y").set_value(y);
    rect.append_attribute("width").set_value(1);
    rect.append_attribute("height").set_value(1);
    rect.append_attribute("fill").set_value(colorHex.c_str());
}

void wpp::SvgRaster::Save(const char *path) {
    doc.save_file(path);
}
