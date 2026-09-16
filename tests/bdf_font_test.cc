// Font-rendering test for the real fonts/tom-thumb.bdf file. Every expected
// value here was hand-derived from that file's actual BDF data (see the
// comments below) -- nothing is a guess or a mock font.
//
// Run via: ctest --test-dir build

#include "watchpanel/graphics_context.h"
#include "watchpanel/raster.h"
#include "watchpanel/bdf_font.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Records every SetPixel call into a plain grid so a rendered frame can be
// compared as human-readable ASCII art (space = off, '*' = on).
class FakeRaster : public watchpanel::Raster {
public:
    FakeRaster(int width, int height)
        : width_(width), height_(height), lit_(width * height, false) {}

    int Width() const override { return width_; }
    int Height() const override { return height_; }

    void Clear() override {
        std::fill(lit_.begin(), lit_.end(), false);
    }

    void SetPixel(int x, int y, watchpanel::Color) override {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        lit_[y * width_ + x] = true;
    }

    // Joins each row into one '\n'-separated string, so it can be compared
    // directly against an inline ASCII picture.
    std::string Render() const {
        std::string out;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                out += lit_[y * width_ + x] ? '*' : ' ';
            }
            if (y + 1 < height_) out += '\n';
        }
        return out;
    }

private:
    int width_;
    int height_;
    std::vector<bool> lit_;
};

std::string Join(const std::vector<std::string> &rows) {
    std::string out;
    for (size_t i = 0; i < rows.size(); ++i) {
        out += rows[i];
        if (i + 1 < rows.size()) out += '\n';
    }
    return out;
}

int failures = 0;

void Check(bool condition, const char *what) {
    if (!condition) {
        std::cerr << "FAILED: " << what << std::endl;
        ++failures;
    }
}

}  // namespace

int main() {
    using namespace watchpanel;

    // --- Metadata sanity checks, straight from fonts/tom-thumb.bdf ---
    // (grep -n "^FONT_ASCENT\|^FONT_DESCENT" fonts/tom-thumb.bdf -> 5, 1)
    const BdfFont &font = BdfFont::Load("fonts/tom-thumb.bdf");
    Check(font.IsLoaded(), "font loads");
    Check(font.Ascent() == 5, "font ascent is 5");
    Check(font.Descent() == 1, "font descent is 1");

    // 'A' (BBX 3 5 0 0): sits flush on the baseline.
    const BdfGlyph *a = font.Find('A');
    Check(a != nullptr, "'A' glyph exists");
    Check(a && a->yOffset == 0, "'A' has no descender (yOffset 0)");
    Check(a && a->width == 3, "'A' ink width is 3");
    Check(a && a->dwidth == 4, "'A' advance (DWIDTH) is 4");

    // 'p' (BBX 3 5 0 -1): a descender -- its box starts 1px below baseline.
    const BdfGlyph *p = font.Find('p');
    Check(p != nullptr, "'p' glyph exists");
    Check(p && p->yOffset == -1, "'p' has a descender (yOffset -1)");
    Check(p && p->dwidth == 4, "'p' advance (DWIDTH) is 4, same as 'A'");

    // 'i' and '.' are narrower in ink than 'A', but must still advance by
    // the same DWIDTH -- proves spacing uses advance width, not ink width.
    const BdfGlyph *i = font.Find('i');
    Check(i != nullptr, "'i' glyph exists");
    Check(i && i->width == 1, "'i' ink width is only 1");
    Check(i && i->dwidth == 4, "'i' advance (DWIDTH) is still 4");

    // --- Render "Ap" through the real pipeline and compare as ASCII art ---
    //
    // Hand-derived from the real bitmap rows (BBX + BITMAP hex, decoded
    // MSB-first) and the baseline algorithm (baselineY = lineTop + ascent,
    // glyph drawn at (x + xOffset, baselineY - yOffset - height)):
    //
    //   'A' BBX 3 5 0 0,  rows 40 A0 E0 A0 A0 ->  " * " / "* *" / "***" / "* *" / "* *"
    //   'p' BBX 3 5 0 -1, rows C0 A0 A0 C0 80 ->  "** " / "* *" / "* *" / "** " / "*  "
    //
    // With ascent=5, 'A' (yOffset 0) occupies screen rows 0-4 starting at
    // col 0; advancing by DWIDTH(4)+letterSpacing(1)=5 puts 'p' at col 5,
    // and its yOffset(-1) shifts it one row lower (rows 1-5) -- visibly
    // dropping the descender below where 'A' sits.
    FakeRaster raster(8, 6);
    GraphicsContext context(&raster, "fonts/tom-thumb.bdf");
    TextSpan span("Ap");
    context.DrawText(&span, "tom-thumb", Color(255, 255, 255),
                      /*x=*/0, /*y=*/0, /*width=*/0, /*height=*/0,
                      /*letterSpacing=*/1, /*lineOffset=*/0,
                      Wrap::kNone, Overflow::kVisible);

    // NOTE: trailing spaces in each row below are significant (space = off).
    const std::vector<std::string> expectedRows = {
        " *      ",
        "* *  ** ",
        "***  * *",
        "* *  * *",
        "* *  ** ",
        "     *  ",
    };
    const std::string expected = Join(expectedRows);
    const std::string actual = raster.Render();

    if (actual != expected) {
        std::cerr << "FAILED: \"Ap\" rendered mismatch\nGot:\n" << actual
                  << "\n\nExpected:\n" << expected << std::endl;
        ++failures;
    }

    if (failures == 0) {
        std::cout << "OK (" << "font+baseline checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
