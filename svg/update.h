#ifndef WATCHPANEL_UPDATE_H_
#define WATCHPANEL_UPDATE_H_

#include "strings.h"
#include "graphics.h"

namespace watchpanel {

    class Updateable {
    public:

        virtual ~Updateable();
        
        virtual void Update(const std::map<std::string, std::string> &vars) {}

    };

    class UpdateText : public Updateable {
    private:

        FormattedString formatter;
        TextGraphic &graphic;

    public:

        UpdateText(const char * text, TextGraphic &graphic);
        ~UpdateText();
    
        void Update(const std::map<std::string, std::string> &vars);

    };

}

#endif // WATCHPANEL_UPDATE_H_