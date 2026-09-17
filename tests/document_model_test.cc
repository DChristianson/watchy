// DocumentModel / FormattedString test, fixtured on a real captured
// OpenWeatherMap response (tests/fixtures/weather_data.json) rather than
// synthetic JSON -- same "test against real data" approach as
// bdf_font_test.cc.
//
// Run via: ctest --test-dir build

#include "watchpanel/model.h"
#include "watchpanel/strings.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include <cstdio>
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

void CheckEq(const std::string &actual, const std::string &expected, const char *what) {
    if (actual != expected) {
        std::cerr << "FAILED: " << what << " -- got [" << actual << "], expected [" << expected << "]"
                   << std::endl;
        ++failures;
    }
}

}  // namespace

int main() {
    using namespace watchpanel;

    const char *path = "tests/fixtures/weather_data.json";
    FILE *file = fopen(path, "rb");
    Check(file != nullptr, "fixture file opens");
    if (!file) {
        std::cerr << failures << " check(s) failed" << std::endl;
        return 1;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(file);
    Check(!doc.HasParseError(), "fixture parses as JSON");

    DocumentModel model(doc);
    std::string value;

    // --- DocumentModel::Find: direct JSON-pointer lookups against the
    // real captured payload ---
    Check(model.Find("/name", value) && value == "Seattle", "/name -> \"Seattle\"");
    Check(model.Find("/weather/0/main", value) && value == "Clouds", "/weather/0/main -> \"Clouds\"");
    Check(model.Find("/weather/0/icon", value) && value == "04n", "/weather/0/icon -> \"04n\"");

    // Integers take a different (exact) formatting path than doubles --
    // no floating point involved, so these are fully deterministic.
    Check(model.Find("/main/pressure", value) && value == "1019", "/main/pressure -> \"1019\" (integer)");
    Check(model.Find("/main/humidity", value) && value == "84", "/main/humidity -> \"84\" (integer)");
    Check(model.Find("/id", value) && value == "5809844", "/id -> \"5809844\" (integer)");

    // A missing pointer must fail cleanly -- no throw, no fabricated value,
    // and Find leaves its output argument untouched.
    Check(!model.Find("/main/does_not_exist", value), "/main/does_not_exist -> not found");
    Check(!model.Find("/nope/nope/nope", value), "deeply missing pointer -> not found");

    // Doubles go through std::to_string(double) (fixed notation, 6
    // fractional digits) -- e.g. the fixture's 288.19 becomes "288.190000",
    // not "288.19". Checked by running this and reading the real output,
    // not assumed.
    std::string temp;
    Check(model.Find("/main/temp", temp), "/main/temp found");
    CheckEq(temp, "288.190000", "/main/temp double formatting");

    // --- FormattedString::Format: template substitution against the same
    // real document ---
    FormattedString greeting("{/name} is {/weather/0/description}");
    std::string formatted;
    greeting.Format(model, formatted);
    CheckEq(formatted, "Seattle is broken clouds", "template substitution");

    // A template with two placeholders where the first resolves and the
    // second doesn't: the second must contribute nothing, not silently
    // reuse the first placeholder's value (Format() reuses one local
    // `search` string across placeholders -- confirmed this doesn't leak
    // a stale value forward when a later Find() fails).
    FormattedString mixed("{/name} then {/does/not/exist} end");
    std::string mixedResult;
    mixed.Format(model, mixedResult);
    CheckEq(mixedResult, "Seattle then  end", "unresolved placeholder doesn't reuse a prior value");

    if (failures == 0) {
        std::cout << "OK (DocumentModel/FormattedString checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
