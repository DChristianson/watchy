#ifndef WATCHPANEL_STRINGS_H_
#define WATCHPANEL_STRINGS_H_

#include <stdlib.h>
#include <string>
#include <vector>

#include "model.h"

namespace watchpanel {

    class FormattedString {
    private:

        std::vector<char> formatData;
        std::vector< std::pair<int, int> > vranges;
    
    public:

        FormattedString(const char *templateStr);
        ~FormattedString();

        void Format(const Model &lookup, std::string &result);

        static bool IsTemplatized(const char *text);

    };

}

#endif // WATCHPANEL_STRINGS_H_