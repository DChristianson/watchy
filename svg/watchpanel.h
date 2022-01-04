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

        std::vector<DataImport *> dataList;
        std::vector<Updateable *> updateList;
        std::vector<Graphic *> displayList;
        
        Canvas *canvas;

        void LoadText(const char *text, TextGraphic *textGraphic);

    public:

        WatchPage(Canvas * canvas);
        ~WatchPage();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

    class WatchPanel {
    private:

        std::vector<WatchPage *> pageList;
        int currentPage;
        long lastUpdate;
        long pageInterval;
        long updateInterval;

    public:

        WatchPanel(Canvas * canvas);
        ~WatchPanel();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

}

#endif // WATCHPANEL_H_