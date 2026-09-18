#ifndef WATCHPANEL_UPDATE_H_
#define WATCHPANEL_UPDATE_H_

#include "strings.h"

#include <functional>

namespace watchpanel {

    class Updateable {
    public:

        virtual ~Updateable();

        // `now` is the current time (caller-supplied, not read from the
        // system clock here) and `deltaSeconds` the time elapsed since the
        // previous update. Each Updateable applies its own policy against
        // these -- e.g. deciding whether enough time has passed to refetch
        // or advance -- which is what makes the whole pipeline testable
        // with fixed, deterministic timestamps instead of real sleeps.
        virtual void Update(const Model &lookup, long now, long deltaSeconds) {}

    };

    class UpdateFormattedString : public Updateable {
    private:

        FormattedString formatter;
        std::function<void(const char *)> setter;

    public:

        UpdateFormattedString(const char *text, std::function<void(const char *)> setter);
        ~UpdateFormattedString();

        void Update(const Model &lookup, long now, long deltaSeconds);

    };

}

#endif // WATCHPANEL_UPDATE_H_
