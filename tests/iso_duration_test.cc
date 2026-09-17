// Test for the limited ISO-8601 duration parser used for cache TTLs.
// Every expected value here is a plain, hand-computable unit conversion
// (minutes/hours/days to seconds), not a guess about library behavior.
//
// Run via: ctest --test-dir build

#include "watchpanel/iso_duration.h"

#include <iostream>

namespace {

int failures = 0;

void CheckParses(const char *iso, long expectedSeconds, const char *what) {
    long seconds = -1;
    const bool ok = watchpanel::ParseIsoDuration(iso, &seconds);
    if (!ok || seconds != expectedSeconds) {
        std::cerr << "FAILED: " << what << " -- ParseIsoDuration(\"" << iso << "\") got ok=" << ok
                   << " seconds=" << seconds << ", expected ok=true seconds=" << expectedSeconds
                   << std::endl;
        ++failures;
    }
}

void CheckRejects(const char *iso, const char *what) {
    long seconds = -1;
    if (watchpanel::ParseIsoDuration(iso, &seconds)) {
        std::cerr << "FAILED: " << what << " -- ParseIsoDuration(\"" << (iso ? iso : "(null)")
                   << "\") unexpectedly succeeded with seconds=" << seconds << std::endl;
        ++failures;
    }
}

}  // namespace

int main() {
    CheckParses("PT15M", 15 * 60, "15 minutes");
    CheckParses("P1D", 24 * 60 * 60, "1 day");
    CheckParses("PT24H", 24 * 60 * 60, "24 hours equals 1 day");
    CheckParses("PT1S", 1, "1 second");
    CheckParses("P1DT2H30M", 24 * 60 * 60 + 2 * 60 * 60 + 30 * 60, "1 day 2h30m combined");
    CheckParses("P0D", 0, "zero duration is valid");

    CheckRejects(nullptr, "null input");
    CheckRejects("", "empty string");
    CheckRejects("15M", "missing leading P");
    CheckRejects("P", "bare P with no components");
    CheckRejects("PT", "bare PT with no components");
    CheckRejects("P1Y", "years are unsupported (not a fixed number of seconds)");
    CheckRejects("P1M", "bare month before T is unsupported/ambiguous");
    CheckRejects("PT15Mxyz", "trailing garbage");
    CheckRejects("P1D5S", "time component without a T separator");
    CheckRejects("PTM", "unit letter with no preceding number");

    if (failures == 0) {
        std::cout << "OK (iso_duration checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
