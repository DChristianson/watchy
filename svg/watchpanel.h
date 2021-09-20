#ifndef WATCHPANEL_H_
#define WATCHPANEL_H_

#include "graphics.h"
#include <vector>

namespace watchpanel {

    class Graphic {
    public:
        virtual ~Graphic();
        
        virtual void Draw(rgbmatrix::Canvas * canvas) {}

    };

    class TextGraphic : public Graphic {
    private:

        const char * textPtr;
        const rgbmatrix::Font * font;
        rgbmatrix::Color color;
        int x;
        int y;
        int letter_spacing;
        int line_offset;

    public:

        TextGraphic(
            const char * textPtr,
            const rgbmatrix::Font * font,
            rgbmatrix::Color color,
            int x,
            int y,
            int letter_spacing,
            int line_offset);
        ~TextGraphic();

        void Draw(rgbmatrix::Canvas * canvas);

    };

    class RectGraphic: public Graphic {
    public:

        RectGraphic();
        ~RectGraphic();

        void Draw(rgbmatrix::Canvas * canvas);

    };

    class ImageGraphic : public Graphic {
    public:

        ImageGraphic();
        ~ImageGraphic();

        void Draw(rgbmatrix::Canvas * canvas);

    };

    class DataManager {
    private:
        std::vector<char> buffer;

    public:
        DataManager();
        ~DataManager();

        const char * VariableFormatText(const char *formatStr);

    };

    class WatchPage {
    private:
        DataManager data_manager;
        std::vector<Graphic *> display_list;
        std::vector<std::string> errors;

    public:

        WatchPage();
        ~WatchPage();

        void Clear();
        int LoadPage(const char * path);
        void Draw(rgbmatrix::Canvas * canvas);

    };



}

#endif // WATCHPANEL_H_