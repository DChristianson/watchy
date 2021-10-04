#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics.h"
#include "data.h"
#include "update.h"

#include <vector>

namespace watchpanel {

    class WatchPage {
    private:
    
        std::vector<std::string> errors;

        std::vector<DataImport *> data_list;
        std::vector<Updateable *> update_list;
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