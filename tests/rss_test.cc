// Test for ParseRssFile (RSS 2.0 -> JSON conversion), fixtured on a real
// synthetic RSS document (tests/fixtures/sample_rss.xml -- structurally the
// same shape as a real feed like BBC's, but original placeholder content,
// not copied from any real source).
//
// Run via: ctest --test-dir build

#include "watchpanel/rss.h"
#include "watchpanel/model.h"

#include <iostream>

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

    rapidjson::Document doc;
    Check(ParseRssFile("tests/fixtures/sample_rss.xml", doc), "sample RSS fixture parses");

    DocumentModel model(doc);
    std::string value;

    // Channel-level fields (siblings of <item>, extracted generically).
    Check(model.Find("/channel/title", value) && value == "Sample Feed", "/channel/title");
    Check(model.Find("/channel/description", value) &&
              value == "A synthetic RSS fixture for testing, not real news content",
          "/channel/description (CDATA unwrapped)");
    Check(model.Find("/channel/link", value) && value == "https://example.invalid/feed", "/channel/link");

    // Items array, with per-item fields.
    Check(model.GetArraySize("/channel/items") == 2, "two items parsed");
    Check(model.Find("/channel/items/0/title", value) && value == "First headline", "item 0 title");
    Check(model.Find("/channel/items/0/description", value) && value == "Description of the first item",
          "item 0 description");
    Check(model.Find("/channel/items/0/pubDate", value) && value == "Mon, 01 Jan 2024 00:00:00 GMT",
          "item 0 pubDate");
    Check(model.Find("/channel/items/1/title", value) && value == "Second headline", "item 1 title");

    // --- Negative cases ---
    rapidjson::Document notRss;
    Check(!ParseRssFile("tests/fixtures/weather_data.json", notRss), "non-RSS (plain JSON) file is rejected");

    rapidjson::Document missing;
    Check(!ParseRssFile("tests/fixtures/does_not_exist.xml", missing), "missing file is rejected");

    if (failures == 0) {
        std::cout << "OK (rss checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
