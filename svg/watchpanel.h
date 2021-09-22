#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics.h"
#include "data.h"
#include <vector>
#include <map>

namespace watchpanel {

    class WatchPage {
    private:
    
        std::map<std::string, std::string> vars;

        std::vector<std::string> errors;

        std::vector<DataImport *> data_list;

        std::vector<Graphic *> display_list;

    public:

        WatchPage();
        ~WatchPage();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw(rgbmatrix::Canvas * canvas);

    };


}

#endif // WATCHPANEL_H_