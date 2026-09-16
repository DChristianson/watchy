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
    // Position of the bitmap's lower-left pixel relative to the glyph
    // origin (on the baseline), per the BDF BBX line. A negative yOffset
    // means the glyph has a descender that drops below the baseline.
    int xOffset = 0;
    int yOffset = 0;
    // Horizontal advance to the next glyph's origin, per the BDF DWIDTH
    // line. This is the real character spacing — independent of (and
    // often wider than) the glyph's own ink width above.
    int dwidth = 0;
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

    // Font-wide baseline metrics (from FONT_ASCENT/FONT_DESCENT), used to
    // place each glyph's bitmap relative to a line's baseline rather than
    // its top.
    int Ascent() const { return ascent; }
    int Descent() const { return descent; }

private:

    bool loaded = false;
    int ascent = 0;
    int descent = 0;
    std::map<int, BdfGlyph> glyphs;

    void ParseFile(const std::string &path);

};

}  // namespace watchpanel

#endif  // WATCHPANEL_BDF_FONT_H_
