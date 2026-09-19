#ifndef WATCHPANEL_TRANSITION_H_
#define WATCHPANEL_TRANSITION_H_

#include "graphics.h"
#include "graphics_context.h"
#include "pixel_buffer.h"
#include "update.h"

#include <string>
#include <vector>

namespace watchpanel {

    // A generic graphic that shows one child graphic, holds it, then does a
    // true per-pixel cross-fade into a second child graphic, and holds on
    // the second one indefinitely afterward (a one-shot transition, not a
    // loop). Not tied to any specific use -- e.g. a page's attribution
    // splash fading into its real display list is one consumer, but
    // anything needing two graphics to hand off to each other can use it.
    //
    // A true cross-fade needs both sides' actual pixel colors at once, and
    // Raster (the real display backend) is write-only, so this owns two
    // off-screen PixelBuffers -- one GraphicsContext each -- and blends
    // their pixels into the real context's raster every Draw(). The child
    // graphics must therefore be constructed against FromContext()/
    // ToContext(), not the page's own context.
    class FadeTransitionGraphic : public Graphic, public Updateable {
    private:

        int x;
        int y;
        int width;
        int height;
        PixelBuffer fromBuffer;
        PixelBuffer toBuffer;
        GraphicsContext fromContext;
        GraphicsContext toContext;
        Graphic *fromGraphic;
        Graphic *toGraphic;
        long holdSeconds;
        long fadeSeconds;
        long elapsedSeconds;
        std::vector<Updateable *> childUpdates;

    public:

        // fontPath/cacheDir are forwarded to the two internal
        // GraphicsContexts (see GraphicsContext's constructor) so text/
        // image children of the transition render exactly like they would
        // on the real context.
        FadeTransitionGraphic(
            GraphicsContext *context,
            int x,
            int y,
            int width,
            int height,
            long holdSeconds,
            long fadeSeconds,
            const std::string &fontPath,
            const std::string &cacheDir);
        ~FadeTransitionGraphic();

        // Child graphics are constructed by the caller against
        // FromContext()/ToContext() and handed over here; ownership
        // transfers to the FadeTransitionGraphic.
        GraphicsContext *FromContext() { return &fromContext; }
        GraphicsContext *ToContext() { return &toContext; }
        void SetFromGraphic(Graphic *graphic);
        void SetToGraphic(Graphic *graphic);
        void AddChildUpdate(Updateable *update);

        // 0 before holdSeconds has elapsed, 1 once the fade has fully
        // completed (and forever after), linear in between. Exposed
        // mainly so tests don't have to reverse-engineer it from pixels.
        double Progress() const;

        void Update(const Model &model, long now, long deltaSeconds);
        void Draw();

    };

}

#endif // WATCHPANEL_TRANSITION_H_
