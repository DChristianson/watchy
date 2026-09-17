#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics_context.h"
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

        GraphicsContext *context;
        std::string configPath;
        std::string secretsPath;
        std::string cacheDir;

        void LoadText(const char *text, TextGraphic *textGraphic);

    public:

        WatchPage(GraphicsContext * context,
                  const std::string &configPath = "config.json",
                  const std::string &secretsPath = "secrets.json",
                  const std::string &cacheDir = "cache");
        ~WatchPage();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

    class WatchPanel {
    private:

        std::vector<WatchPage *> pageList;
        GraphicsContext *context;
        std::string configPath;
        std::string secretsPath;
        std::string cacheDir;
        int currentPage;
        long lastUpdate;
        long lastPageFlip;
        long pageInterval;
        long updateInterval;

    public:

        WatchPanel(GraphicsContext * context,
                   const std::string &configPath = "config.json",
                   const std::string &secretsPath = "secrets.json",
                   const std::string &cacheDir = "cache");
        ~WatchPanel();

        int Load(const char * path);
        void Clear();

        void Update();
        void Draw();

    };

}

#endif // WATCHPANEL_H_