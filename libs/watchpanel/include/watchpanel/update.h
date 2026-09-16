#ifndef WATCHPANEL_UPDATE_H_
#define WATCHPANEL_UPDATE_H_

#include "strings.h"

#include <functional>

namespace watchpanel {

    class Updateable {
    public:

        virtual ~Updateable();
        
        virtual void Update(const Model &lookup) {}

    };

    class UpdateFormattedString : public Updateable {
    private:

        FormattedString formatter;
        std::function<void(const char *)> setter;

    public:

        UpdateFormattedString(const char *text, std::function<void(const char *)> setter);
        ~UpdateFormattedString();
    
        void Update(const Model &lookup);

    };

}

#endif // WATCHPANEL_UPDATE_H_