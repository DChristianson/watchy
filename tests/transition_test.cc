// Test for FadeTransitionGraphic: a generic true per-pixel cross-fade
// between two graphics, driven by fixed synthetic timestamps (no real
// sleeping), same "deterministic Update(now, deltaSeconds)" approach as
// flip_test.cc.
//
// Run via: ctest --test-dir build

#include "watchpanel/graphics.h"
#include "watchpanel/model.h"
#include "watchpanel/transition.h"

#include "fake_raster.h"

#include "rapidjson/document.h"

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

}  // namespace

int main() {
    using namespace watchpanel;

    rapidjson::Document doc;
    doc.Parse("{}");
    DocumentModel model(doc);

    watchy_test::FakeRaster raster(4, 4);
    GraphicsContext context(&raster, "");  // no font needed -- both children are plain rects

    // holdSeconds=2, fadeSeconds=4: colors chosen (0,0,0) -> (200,100,40)
    // so every intermediate fraction used below lands on an exact integer,
    // with no floating-point rounding ambiguity to account for.
    const long holdSeconds = 2;
    const long fadeSeconds = 4;
    FadeTransitionGraphic transition(&context, 0, 0, 4, 4, holdSeconds, fadeSeconds, "", "");

    const Color fromColor(0, 0, 0);
    const Color toColor(200, 100, 40);
    transition.SetFromGraphic(new RectGraphic(transition.FromContext(), 0, 0, 4, 4, fromColor, fromColor));
    transition.SetToGraphic(new RectGraphic(transition.ToContext(), 0, 0, 4, 4, toColor, toColor));

    auto drawAndSample = [&]() {
        raster.Clear();
        transition.Draw();
        return raster.ColorAt(0, 0);
    };

    // --- Hold phase: full "from" color, no fade yet ---
    transition.Update(model, 1000, 0);  // elapsed=0
    Check(transition.Progress() == 0.0, "progress is 0 at the very start");
    CheckColorEq(drawAndSample(), fromColor, "shows the from-color during the hold phase (t=0)");

    transition.Update(model, 1001, 1);  // elapsed=1, still <= holdSeconds(2)
    Check(transition.Progress() == 0.0, "progress stays 0 while still within the hold duration");
    CheckColorEq(drawAndSample(), fromColor, "still the from-color just before the hold ends");

    // --- Fade phase: linear cross-fade, checked at exact fractions ---
    transition.Update(model, 1003, 2);  // elapsed=3 -> 1s into a 4s fade -> t=0.25
    Check(transition.Progress() == 0.25, "progress is 0.25 a quarter through the fade");
    CheckColorEq(drawAndSample(), Color(50, 25, 10), "pixel is 25% of the way from from-color to to-color");

    transition.Update(model, 1005, 2);  // elapsed=5 -> 3s into the fade -> t=0.75
    Check(transition.Progress() == 0.75, "progress is 0.75 three quarters through the fade");
    CheckColorEq(drawAndSample(), Color(150, 75, 30), "pixel is 75% of the way from from-color to to-color");

    // --- Fully faded: exactly the to-color, and it stays there ---
    transition.Update(model, 1006, 1);  // elapsed=6 == hold+fade: fully faded
    Check(transition.Progress() == 1.0, "progress reaches exactly 1 once hold+fade seconds have elapsed");
    CheckColorEq(drawAndSample(), toColor, "pixel is exactly the to-color once fully faded");

    transition.Update(model, 1010, 4);  // elapsed=10, well past the fade
    Check(transition.Progress() == 1.0, "progress clamps at 1 rather than continuing past 1");
    CheckColorEq(drawAndSample(), toColor, "stays at the to-color indefinitely after the fade completes");

    if (failures == 0) {
        std::cout << "OK (fade transition checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
