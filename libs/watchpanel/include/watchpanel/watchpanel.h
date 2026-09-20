#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics_context.h"
#include "data.h"
#include "transition.h"
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
        // Non-owning aliases into displayList (FlipGraphic instances,
        // which are also Updateable) -- displayList's Clear() owns and
        // deletes the actual objects.
        std::vector<Updateable *> flipUpdates;

        // Set only when the page has an <attribution>: the whole display
        // list is then built against pageTransition->ToContext() instead
        // of context directly and wrapped in a GroupGraphic as its "to"
        // side, so displayList becomes a non-owning alias list too (see
        // Clear()) and Draw()/Update() defer to pageTransition instead of
        // walking displayList themselves.
        FadeTransitionGraphic *pageTransition;

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

        void Update(long now, long deltaSeconds);
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
        long lastPageFlip;
        long pageInterval;

    public:

        WatchPanel(GraphicsContext * context,
                   const std::string &configPath = "config.json",
                   const std::string &secretsPath = "secrets.json",
                   const std::string &cacheDir = "cache");
        ~WatchPanel();

        int Load(const char * path);
        void Clear();

        void Update(long now, long deltaSeconds);
        void Draw();

    };

}

#endif // WATCHPANEL_H_