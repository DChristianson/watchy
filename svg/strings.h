#ifndef WATCHPANEL_STRINGS_H_
#define WATCHPANEL_STRINGS_H_

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

namespace watchpanel {

    
    class FormattedString {
    private:

        const std::map<std::string, std::string> &vars;
        std::vector<char> formatData;
        std::vector< std::pair<int, int> > vranges;
        std::string value;
    
    public:

        FormattedString(const char *templateStr, const std::map<std::string, std::string> &vars);
        ~FormattedString();

        void Update();
        const char *Value();

    };

}

#endif // WATCHPANEL_STRINGS_H_