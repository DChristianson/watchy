#include "watchpanel/bdf_font.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace watchpanel {
namespace {

bool StartsWith(const std::string &s, const char *prefix) {
    const std::string p(prefix);
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

}  // namespace

bool BdfGlyph::GetBit(int x, int y) const {
    if (y < 0 || y >= static_cast<int>(rows.size())) return false;
    return ((rows[y] >> (storageWidth - 1 - x)) & 0x1U) != 0;
}

void BdfFont::ParseFile(const std::string &path) {
    std::ifstream in(path.c_str());
    if (!in.is_open()) return;

    int encoding = -1;
    int width = 0;
    int height = 0;
    int storageWidth = 0;
    bool inBitmap = false;
    std::vector<unsigned int> rows;
    std::string line;

    while (std::getline(in, line)) {
        if (StartsWith(line, "STARTCHAR")) {
            encoding = -1;
            width = 0;
            height = 0;
            storageWidth = 0;
            rows.clear();
            inBitmap = false;
            continue;
        }
        if (StartsWith(line, "ENCODING ")) {
            encoding = std::atoi(line.substr(9).c_str());
            continue;
        }
        if (StartsWith(line, "BBX ")) {
            std::istringstream ss(line.substr(4));
            int xoff = 0;
            int yoff = 0;
            ss >> width >> height >> xoff >> yoff;
            storageWidth = ((width + 7) / 8) * 8;
            continue;
        }
        if (line == "BITMAP") {
            inBitmap = true;
            continue;
        }
        if (line == "ENDCHAR") {
            if (encoding >= 0 && width > 0 && height > 0 && !rows.empty()) {
                BdfGlyph g;
                g.width = width;
                g.height = height;
                g.storageWidth = storageWidth;
                g.rows = rows;
                glyphs[encoding] = g;
            }
            inBitmap = false;
            continue;
        }
        if (inBitmap) {
            unsigned int row = 0;
            std::istringstream hs(line);
            hs >> std::hex >> row;
            rows.push_back(row);
        }
    }

    loaded = !glyphs.empty();
}

const BdfFont &BdfFont::Load(const std::string &path) {
    static std::map<std::string, BdfFont> cache;
    auto it = cache.find(path);
    if (it != cache.end()) return it->second;
    BdfFont &font = cache[path];
    font.ParseFile(path);
    return font;
}

const BdfGlyph *BdfFont::Find(int code) const {
    auto it = glyphs.find(code);
    if (it == glyphs.end()) return nullptr;
    return &it->second;
}

}  // namespace watchpanel
