// End-to-end test for <attribution>: a page-level splash that shows on
// load, then true-cross-fades into the page's real display list. Uses a
// small self-contained XML page (no network, no feeds) written to a
// generated fixture file, same approach as weather_page_test.cc.
//
// The attribution and the page's real content both render the same glyph
// ('A', from fonts/tom-thumb.bdf) at the same position but in different
// colors, so a single pixel's color over time traces out the whole
// transition: pure attribution color during the hold, a computable blend
// during the fade, and pure page color once it completes.
//
// Run via: ctest --test-dir build

#include "watchpanel/watchpanel.h"
#include "fake_raster.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void Check(bool condition, const char *what) {
    if (!condition) {
        std::cerr << "FAILED: " << what << std::endl;
        ++failures;
    }
}

void CheckColorEq(watchpanel::Color actual, watchpanel::Color expected, const char *what) {
    if (actual != expected) {
        std::cerr << "FAILED: " << what << " -- got (" << (int)actual.r << "," << (int)actual.g << ","
                   << (int)actual.b << "), expected (" << (int)expected.r << "," << (int)expected.g << ","
                   << (int)expected.b << ")" << std::endl;
        ++failures;
    }
}

const char *kPageXml = R"XML(
<page>
    <attribution hold="PT2S" fade="PT4S" font="tom-thumb" color="red" x="0" y="0">
        <tspan>A</tspan>
    </attribution>
    <display>
        <text x="0" y="0" font="tom-thumb" color="#00c828">
            <tspan>A</tspan>
        </text>
    </display>
</page>
)XML";

}  // namespace

int main() {
    using namespace watchpanel;

    const std::string generatedPath = "tests/fixtures/attribution_test.generated.xml";
    std::ofstream out(generatedPath, std::ios::binary);
    out << kPageXml;
    out.close();

    watchy_test::FakeRaster raster(8, 8);
    GraphicsContext context(&raster, "fonts/tom-thumb.bdf");
    WatchPage page(&context, "configs/runtime/config.json", "configs/runtime/secrets.json",
                   "tests/fixtures/.attribution_test_cache");
    const int loadResult = page.Load(generatedPath.c_str());
    std::remove(generatedPath.c_str());
    Check(loadResult == 0, "attribution_test.xml loads");

    // 'A' (BBX 3 5 0 0, row 40 -> " * ") lights column 1, row 0 -- a pixel
    // both the attribution and the real page draw identically positioned,
    // so it's exactly this transition's blended value at every point in
    // time. from=red(255,0,0), to=#00c828(0,200,40).
    const Color fromColor(255, 0, 0);
    const Color toColor(0, 200, 40);
    auto drawAndSample = [&]() {
        raster.Clear();
        page.Draw();
        return raster.ColorAt(1, 0);
    };

    // --- Hold phase (2s): pure attribution color ---
    page.Update(1000, 0);
    CheckColorEq(drawAndSample(), fromColor, "shows the attribution color during the hold phase");

    page.Update(1001, 1);  // elapsed=1, still within the 2s hold
    CheckColorEq(drawAndSample(), fromColor, "still the attribution color just before the hold ends");

    // --- Fade phase (4s): linear cross-fade, checked at exact fractions
    // (255*0.25=63.75, 200*0.25=50.0, 40*0.25=10.0 -- all exactly
    // representable, so the truncating cast has a single unambiguous
    // answer) ---
    page.Update(1003, 2);  // elapsed=3 -> 1s into the 4s fade -> t=0.25
    CheckColorEq(drawAndSample(), Color(191, 50, 10), "pixel is 25% faded from attribution color to page color");

    page.Update(1005, 2);  // elapsed=5 -> 3s into the fade -> t=0.75
    CheckColorEq(drawAndSample(), Color(63, 150, 30), "pixel is 75% faded from attribution color to page color");

    // --- Fully faded: exactly the real page's color, and it stays there ---
    page.Update(1006, 1);  // elapsed=6 == hold+fade
    CheckColorEq(drawAndSample(), toColor, "pixel is exactly the page's color once fully faded");

    page.Update(1020, 14);  // well past the fade
    CheckColorEq(drawAndSample(), toColor, "stays on the real page indefinitely after the fade completes");

    if (failures == 0) {
        std::cout << "OK (attribution transition checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
