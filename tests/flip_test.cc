// Test for FlipGraphic: cycling through a JSON array over time, scoping
// relative template paths to the currently showing item, and clamping when
// the array shrinks or empties out. Everything runs on fixed, synthetic
// timestamps (no real sleeping) since Update() now takes (now, deltaSeconds)
// explicitly rather than reading the system clock -- that's the whole point
// of the change this test exercises.
//
// Run via: ctest --test-dir build

#include "watchpanel/graphics.h"
#include "watchpanel/graphics_context.h"
#include "watchpanel/model.h"
#include "watchpanel/update.h"

#include "fake_raster.h"

#include "rapidjson/document.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void Check(bool condition, const char *what) {
    if (!condition) {
        std::cerr << "FAILED: " << what << std::endl;
        ++failures;
    }
}

void CheckEq(const std::string &actual, const std::string &expected, const char *what) {
    if (actual != expected) {
        std::cerr << "FAILED: " << what << " -- got [" << actual << "], expected [" << expected << "]"
                   << std::endl;
        ++failures;
    }
}

std::string Join(const std::vector<std::string> &rows) {
    std::string out;
    for (size_t i = 0; i < rows.size(); ++i) {
        out += rows[i];
        if (i + 1 < rows.size()) out += '\n';
    }
    return out;
}

}  // namespace

int main() {
    using namespace watchpanel;

    rapidjson::Document doc;
    doc.Parse(R"JSON({
        "news": { "items": [ {"title": "First"}, {"title": "Second"}, {"title": "Third"} ] },
        "config": { "label": "Headlines" }
    })JSON");
    Check(!doc.HasParseError(), "test fixture JSON parses");
    DocumentModel model(doc);

    watchy_test::FakeRaster raster(64, 16);
    GraphicsContext context(&raster);

    const long period = 10;  // seconds
    FlipGraphic flip(&context, "tom-thumb", Color(255, 255, 255), 0, 0, 64, 16,
                      1, 0, Wrap::kNone, Overflow::kVisible, "/news/items", period);

    // Relative path ("title") should scope to the current item; an
    // absolute path ("/config/label") should reach outside the flip
    // entirely, unaffected by which item is showing.
    TextSpan &titleSpan = flip.AppendText("{title}");
    flip.AddChildUpdate(new UpdateFormattedString("{title}", [&titleSpan](const char *v) { titleSpan.text = v; }));

    TextSpan &labelSpan = flip.AppendText("{/config/label}");
    flip.AddChildUpdate(new UpdateFormattedString("{/config/label}", [&labelSpan](const char *v) { labelSpan.text = v; }));

    // --- Timing: show item 0 immediately, advance only once `period`
    // seconds have actually passed, wrap around after the last item ---
    flip.Update(model, 1000, 0);
    CheckEq(titleSpan.text, "First", "shows item 0 immediately on first update");
    CheckEq(labelSpan.text, "Headlines", "absolute path resolves regardless of flip state");

    flip.Update(model, 1005, 5);  // only 5s elapsed, period is 10s
    CheckEq(titleSpan.text, "First", "doesn't advance before a full period has passed");

    flip.Update(model, 1010, 5);  // now 10s since the last flip: advances
    CheckEq(titleSpan.text, "Second", "advances to item 1 once period seconds have passed");

    flip.Update(model, 1015, 5);
    CheckEq(titleSpan.text, "Second", "doesn't advance again before another full period");

    flip.Update(model, 1020, 5);
    CheckEq(titleSpan.text, "Third", "advances to item 2");

    flip.Update(model, 1030, 10);
    CheckEq(titleSpan.text, "First", "wraps back to item 0 after the last item");
    CheckEq(labelSpan.text, "Headlines", "absolute path still resolves after wrapping");

    // --- Clamp to 0 when the array shrinks out from under the current index ---
    rapidjson::Document shrunk;
    shrunk.Parse(R"JSON({
        "news": { "items": [ {"title": "OnlyOne"} ] },
        "config": { "label": "Headlines" }
    })JSON");
    DocumentModel shrunkModel(shrunk);
    flip.Update(shrunkModel, 1040, 10);
    CheckEq(titleSpan.text, "OnlyOne", "clamps to index 0 when the array shrank past the current index");

    // --- Empty array: no crash, children simply aren't updated ---
    rapidjson::Document empty;
    empty.Parse(R"JSON({ "news": { "items": [] }, "config": { "label": "Headlines" } })JSON");
    DocumentModel emptyModel(empty);
    flip.Update(emptyModel, 1050, 10);
    CheckEq(titleSpan.text, "OnlyOne", "empty items array leaves prior content untouched rather than crashing");

    // --- scroll-speed: vertical auto-scroll when an item's content is
    // taller than the flip's box, stopping (not looping) once fully
    // revealed, and resetting to the top on the next flip advance ---
    //
    // Two literal "A" lines (no template needed -- the content itself
    // doesn't matter here, only that there are two of them). With
    // tom-thumb's ascent(5)/ 'A' height(5) and lineOffset defaulting to
    // maxGlyphHeight+1 = 6, two lines is a 12px-tall block; a 6px-tall box
    // means only one line's worth is visible at a time and maxScroll = 6.
    //
    // Every expected row picture below was hand-derived from 'A''s real
    // BDF bitmap (BBX 3 5 0 0, rows 40 A0 E0 A0 A0 -> " * "/"* *"/"***"/
    // "* *"/"* *", same glyph bdf_font_test.cc already verifies), applying
    // DrawText's scroll math (cursorY = y - scrollOffsetY) by hand.
    rapidjson::Document scrollDoc;
    scrollDoc.Parse(R"JSON({ "news": { "items": [ {"n": 1}, {"n": 2} ] } })JSON");
    Check(!scrollDoc.HasParseError(), "scroll test fixture JSON parses");
    DocumentModel scrollModel(scrollDoc);

    watchy_test::FakeRaster scrollRaster(8, 6);
    GraphicsContext scrollContext(&scrollRaster, "fonts/tom-thumb.bdf");
    const long scrollPeriod = 100;  // long enough not to advance mid-test
    FlipGraphic scrollFlip(&scrollContext, "tom-thumb", Color(255, 255, 255), 0, 0, 8, 6,
                           /*letterSpacing=*/1, /*lineOffset=*/0, Wrap::kNone, Overflow::kVisible,
                           "/news/items", scrollPeriod, /*scrollSpeedPxPerSec=*/3);
    scrollFlip.AppendText("A");
    scrollFlip.AppendText("A");

    const std::string frameAtTop = Join({
        " *      ",
        "* *     ",
        "***     ",
        "* *     ",
        "* *     ",
        "        ",
    });
    const std::string frameAtOffset3 = Join({
        "* *     ",
        "* *     ",
        "        ",
        " *      ",
        "* *     ",
        "***     ",
    });

    // Real callers (WatchPage::Draw's raster, panel-runtime, clock-led) all
    // clear the raster before each frame's Draw() -- do the same here, or
    // successive frames' lit pixels just accumulate on top of each other.
    scrollFlip.Update(scrollModel, 2000, 0);  // first show: scroll starts at 0
    scrollRaster.Clear();
    scrollFlip.Draw();
    CheckEq(scrollRaster.Render(), frameAtTop, "scroll starts at the top when an item is first shown");

    scrollFlip.Update(scrollModel, 2001, 1);  // +3px (speed 3 * 1s)
    scrollRaster.Clear();
    scrollFlip.Draw();
    CheckEq(scrollRaster.Render(), frameAtOffset3, "scrolls up by scrollSpeed*deltaSeconds px");

    scrollFlip.Update(scrollModel, 2002, 1);  // +3px more -> hits maxScroll (6px, i.e. exactly one more line)
    scrollRaster.Clear();
    scrollFlip.Draw();
    CheckEq(scrollRaster.Render(), frameAtTop,
            "fully scrolled: the second line now fills the box, identical in shape to the first line at the top");

    scrollFlip.Update(scrollModel, 2012, 10);  // would be +30px (way past maxScroll) if not clamped
    scrollRaster.Clear();
    scrollFlip.Draw();
    CheckEq(scrollRaster.Render(), frameAtTop, "stops at the end instead of scrolling past/looping");

    scrollFlip.Update(scrollModel, 2200, 188);  // >= period: advances to the next item
    scrollRaster.Clear();
    scrollFlip.Draw();
    CheckEq(scrollRaster.Render(), frameAtTop, "resets scroll to the top when the flip advances to a new item");

    if (failures == 0) {
        std::cout << "OK (flip checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
