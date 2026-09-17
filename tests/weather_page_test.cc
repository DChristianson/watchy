// End-to-end render test for the weather page pipeline: real XML parsing,
// a real feed fetch (via hamper::fetch_url/curl, using a file:// URL so no
// network is touched), real JSON-pointer template substitution against the
// real tests/fixtures/weather_data.json fixture, and a real decode +
// nearest-neighbor resize of tests/fixtures/weather_icon.png -- a small
// synthetic 50x50 PNG generated for this test suite (not real weather-icon
// artwork, which is proprietary to the API and isn't something to commit
// here). Its pixels follow a known formula (r=(x*5)%256, g=(y*5)%256,
// b=128 -- see image_decode_test.cc), which is what makes the exact
// resampled colors checked below computable ground truth, not a guess.
//
// tests/fixtures/weather_test.xml mirrors configs/pages/weather.xml's
// structure (a feed-driven <text> plus a local <image>) but points both at
// local fixtures instead of live network endpoints, via a
// __WEATHER_DATA_FILE_URL__ placeholder this test resolves to an absolute
// file:// URL at run time (curl's file:// scheme needs an absolute path,
// and the source tree can be checked out anywhere).
//
// Run via: ctest --test-dir build

#include "watchpanel/watchpanel.h"
#include "fake_raster.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void Check(bool condition, const char *what) {
    if (!condition) {
        std::cerr << "FAILED: " << what << std::endl;
        ++failures;
    }
}

std::string ReadFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

}  // namespace

int main() {
    using namespace watchpanel;

    // Resolve the fixture to an absolute file:// URL.
    char resolved[4096];
    Check(realpath("tests/fixtures/weather_data.json", resolved) != nullptr,
          "resolved absolute path to weather_data.json fixture");
    const std::string dataUrl = std::string("file://") + resolved;

    std::string xml = ReadFile("tests/fixtures/weather_test.xml");
    const std::string placeholder = "__WEATHER_DATA_FILE_URL__";
    const size_t pos = xml.find(placeholder);
    Check(pos != std::string::npos, "placeholder found in weather_test.xml");
    xml.replace(pos, placeholder.size(), dataUrl);

    const std::string generatedPath = "tests/fixtures/weather_test.generated.xml";
    std::ofstream out(generatedPath, std::ios::binary);
    out << xml;
    out.close();

    watchy_test::FakeRaster raster(64, 64);
    GraphicsContext context(&raster, "fonts/tom-thumb.bdf");
    WatchPage page(&context, "configs/runtime/config.json", "configs/runtime/secrets.json");
    const int loadResult = page.Load(generatedPath.c_str());
    std::remove(generatedPath.c_str());
    Check(loadResult == 0, "weather_test.xml loads");

    page.Update();
    page.Draw();

    // --- Image: real dimension-aware letterboxing ---
    //
    // Box is x=30,y=10,w=30,h=20. The real icon is 50x50 (square), so
    // scale = min(30/50, 20/50) = 0.4 -> a 20x20 square, centered:
    // offsetX = 30 + (30-20)/2 = 35, offsetY = 10 + (20-20)/2 = 10.
    // So pixels should be lit exactly on columns 35-54, rows 10-29 --
    // and NOT on the box's left/right margins (columns 30-34, 55-59),
    // which the old naive full-box placeholder would have lit.
    bool imageBoxCorrect = true;
    for (int y = 10; y < 30; ++y) {
        for (int x = 30; x < 60; ++x) {
            const bool expectedLit = (x >= 35 && x < 55 && y >= 10 && y < 30);
            if (raster.IsLit(x, y) != expectedLit) {
                imageBoxCorrect = false;
            }
        }
    }
    Check(imageBoxCorrect, "image letterboxed to real 50x50 aspect ratio within its 30x20 box");

    // Verify the actual resampled pixel colors, not just which pixels got
    // touched -- proves the real decode + nearest-neighbor resize engaged,
    // not some other code path that happens to light up the same shape.
    // Nearest-neighbor: srcX = floor(dx * 50 / 20), srcY = floor(dy * 50 / 20)
    // for dst offset (dx,dy) within the drawn 20x20 region at (35,10).
    // Source color at (srcX,srcY): r=(srcX*5)%256, g=(srcY*5)%256, b=128.
    auto CheckImagePixel = [&](int dx, int dy, const char *what) {
        const int srcX = (dx * 50) / 20;
        const int srcY = (dy * 50) / 20;
        const Color expected(static_cast<uint8_t>((srcX * 5) % 256),
                              static_cast<uint8_t>((srcY * 5) % 256), 128);
        const Color actual = raster.ColorAt(35 + dx, 10 + dy);
        if (actual != expected) {
            std::cerr << "FAILED: " << what << " -- got (" << int(actual.r) << "," << int(actual.g)
                       << "," << int(actual.b) << "), expected (" << int(expected.r) << ","
                       << int(expected.g) << "," << int(expected.b) << ")" << std::endl;
            ++failures;
        }
    };
    CheckImagePixel(0, 0, "image top-left sample");
    CheckImagePixel(19, 0, "image top-right sample");
    CheckImagePixel(0, 19, "image bottom-left sample");
    CheckImagePixel(19, 19, "image bottom-right sample");
    CheckImagePixel(10, 10, "image center sample");

    // --- Text: the real fetched temp value (288.19 -> "288.190000")
    // rendered through the real font -- verify the first two characters
    // ('2' then '8') match their real BDF bitmaps, hand-decoded from
    // fonts/tom-thumb.bdf (BBX 3 5 0 0, rows C0 20 40 80 E0 / E0 A0 E0 A0
    // E0), placed at (0,0) and (4,0) per DWIDTH=4, ascent=5.
    //   '2' BBX 3 5 0 0, rows C0 20 40 80 E0 -> "** " / "  *" / " * " / "*  " / "***"
    //   '8' BBX 3 5 0 0, rows E0 A0 E0 A0 E0 -> "***" / "* *" / "***" / "* *" / "***"
    // '2' is 3px ink but DWIDTH=4, so '8' starts at column 4, not 3 --
    // column 3 is a 1px gap. Full 7-column span (2's 3 + gap + 8's 3):
    const std::string expected2and8 =
        "**  ***\n"
        "  * * *\n"
        " *  ***\n"
        "*   * *\n"
        "*** ***";
    const std::string actual2and8 = raster.RenderRegion(0, 0, 7, 5);
    if (actual2and8 != expected2and8) {
        std::cerr << "FAILED: temp value's first two digits mismatch\nGot:\n" << actual2and8
                  << "\n\nExpected:\n" << expected2and8 << std::endl;
        ++failures;
    }

    if (failures == 0) {
        std::cout << "OK (weather page render checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
