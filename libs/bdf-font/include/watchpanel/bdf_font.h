#ifndef WATCHPANEL_BDF_FONT_H_
#define WATCHPANEL_BDF_FONT_H_

#include <map>
#include <string>
#include <vector>

namespace watchpanel {

struct BdfGlyph {
    int width = 0;
    int height = 0;
    int storageWidth = 0;
    std::vector<unsigned int> rows;

    bool GetBit(int x, int y) const;
};

// Parses X11 BDF bitmap fonts (e.g. fonts/tom-thumb.bdf). Instances are
// cached by path, so repeated Load() calls for the same font are free.
class BdfFont {
public:

    static const BdfFont &Load(const std::string &path);

    bool IsLoaded() const { return loaded; }
    const BdfGlyph *Find(int code) const;

private:

    bool loaded = false;
    std::map<int, BdfGlyph> glyphs;

    void ParseFile(const std::string &path);

};

}  // namespace watchpanel

#endif  // WATCHPANEL_BDF_FONT_H_
