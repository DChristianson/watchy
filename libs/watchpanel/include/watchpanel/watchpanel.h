#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics.h"
#include "data.h"
#include "update.h"

#include <string>
#include <vector>

namespace watchpanel {

    class WatchPage {
    private:
    
        std::vector<std::string> errors;

        std::vector<DataImport *> dataList;
        std::vector<Updateable *> updateList;
        std::vector<Graphic *> displayList;
        
        Canvas *canvas;
        std::string configPath;
        std::string secretsPath;

        void LoadText(const char *text, TextGraphic *textGraphic);

    public:

        WatchPage(Canvas * canvas,
                  const std::string &configPath = "config.json",
                  const std::string &secretsPath = "secrets.json");
        ~WatchPage();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

    class WatchPanel {
    private:

        std::vector<WatchPage *> pageList;
        Canvas *canvas;
        std::string configPath;
        std::string secretsPath;
        int currentPage;
        long lastUpdate;
        long lastPageFlip;
        long pageInterval;
        long updateInterval;

    public:

        WatchPanel(Canvas * canvas,
                   const std::string &configPath = "config.json",
                   const std::string &secretsPath = "secrets.json");
        ~WatchPanel();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

}

#endif // WATCHPANEL_H_