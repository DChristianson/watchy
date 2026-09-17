#include "watchpanel/iso_duration.h"

namespace watchpanel {

namespace {

// Parses a run of ASCII digits starting at *p, advancing *p past them.
// Returns false if there were no digits at all.
bool ParseNumber(const char **p, long *value) {
    const char *start = *p;
    long v = 0;
    while (**p >= '0' && **p <= '9') {
        v = v * 10 + (**p - '0');
        ++*p;
    }
    if (*p == start) return false;
    *value = v;
    return true;
}

}  // namespace

bool ParseIsoDuration(const char *iso, long *seconds) {
    if (!iso || iso[0] != 'P') return false;

    const char *p = iso + 1;
    long totalSeconds = 0;
    bool sawAnyComponent = false;

    // Date part: only whole days are supported here -- a fixed,
    // unambiguous number of seconds, unlike years or months.
    while (*p != '\0' && *p != 'T') {
        long value = 0;
        if (!ParseNumber(&p, &value)) return false;
        if (*p != 'D') return false;  // Y/M/W (or anything else) here is unsupported
        ++p;
        totalSeconds += value * 86400L;
        sawAnyComponent = true;
    }

    if (*p == 'T') {
        ++p;
        while (*p != '\0') {
            long value = 0;
            if (!ParseNumber(&p, &value)) return false;
            switch (*p) {
                case 'H': totalSeconds += value * 3600L; break;
                case 'M': totalSeconds += value * 60L; break;
                case 'S': totalSeconds += value; break;
                default: return false;
            }
            ++p;
            sawAnyComponent = true;
        }
    }

    if (!sawAnyComponent) return false;  // "P" or "PT" alone isn't a valid duration
    if (*p != '\0') return false;        // trailing garbage

    *seconds = totalSeconds;
    return true;
}

}  // namespace watchpanel
