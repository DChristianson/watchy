#include "strings.h"

#include <cstdio>
#include <cstdlib>

namespace wpp = watchpanel;

wpp::FormattedString::FormattedString(const char *templateStr)
{
    const int size = strlen(templateStr);
    formatData.reserve(size);

    for (int i = 0; i < size; i++) {
        char c = templateStr[i];
        if ('\\' == c && (i + 1) < size) {
            i += 1;
            c = templateStr[i];
        } else if ('{' == c) {
            i += 1;
            if (i < size) {
                formatData.push_back(0);
                int start = formatData.size();
                c = templateStr[i];
                while ('}' != c && i < size) {
                    formatData.push_back(c);
                    i += 1;
                    c = templateStr[i];
                }
                formatData.push_back(0);
                int end = formatData.size();
                vranges.push_back(std::pair<int, int>(start, end));
            }
            continue;
        }
        formatData.push_back(c);
    }
    formatData.push_back(0);
}

wpp::FormattedString::~FormattedString() {}

namespace {

// Splits a trailing ":N" precision spec (N = 1+ digits) off a placeholder's
// variable name, e.g. "/weather/main/temp:0" -> path "/weather/main/temp",
// precision 0. If there's no such suffix (or what follows the last ':'
// isn't purely digits -- e.g. an RSS "dc:creator" path), returns the
// original text unchanged with hasPrecision=false, so existing paths keep
// working exactly as before.
void SplitPrecision(const char *varName, std::string &path, int &precision, bool &hasPrecision) {
    const std::string text(varName);
    const size_t colon = text.rfind(':');
    if (colon != std::string::npos) {
        const std::string suffix = text.substr(colon + 1);
        if (!suffix.empty() && suffix.find_first_not_of("0123456789") == std::string::npos) {
            path = text.substr(0, colon);
            precision = std::atoi(suffix.c_str());
            hasPrecision = true;
            return;
        }
    }
    path = text;
    hasPrecision = false;
}

}  // namespace

void wpp::FormattedString::Format(const Model &lookup, std::string &value) {
    int i = 0;
    value.clear();
    const char * data = formatData.data();
    std::string search;
    for (auto vrange : vranges) {
        if (vrange.first > i) {
            value.append(data + i);
        }
        const char * varName = data + vrange.first;

        std::string path;
        int precision = 0;
        bool hasPrecision = false;
        SplitPrecision(varName, path, precision, hasPrecision);

        if (lookup.Find(path.c_str(), search)) {
            if (hasPrecision) {
                char *end = nullptr;
                const double numeric = std::strtod(search.c_str(), &end);
                if (end != search.c_str() && *end == '\0') {
                    // search was a fully-numeric string -- reformat it at
                    // the requested precision (e.g. "61.740000" -> "62").
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "%.*f", precision, numeric);
                    value.append(buf);
                } else {
                    // Not a number (e.g. a string field with a stray
                    // ":something" suffix) -- the precision spec doesn't
                    // apply; pass the raw value through.
                    value.append(search);
                }
            } else {
                value.append(search);
            }
        }
        i = vrange.second;
    }
    if (formatData.size() > (size_t) i) {
        value.append(data + i);
    }
}

bool wpp::FormattedString::IsTemplatized(const char *value) {
    char c;
    do {
        c = *value;
        if ('{' == c) return true;
        value += 1;
    } while (c != 0);
    return false;
}
