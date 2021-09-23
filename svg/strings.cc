#include "strings.h"
#include <iostream>

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

void wpp::FormattedString::Format(const std::map<std::string, std::string> &vars, std::string &value) {
    int i = 0;
    value.clear();
    const char * data = formatData.data();
    for (auto vrange : vranges) {
        if (vrange.first > i) {
            value.append(data + i);
        }
        const char * varName = data + vrange.first;
        auto search = vars.find(varName);
        if (search != vars.end()) {
            std::cout << varName << std::endl;
            value.append(search->second);
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

