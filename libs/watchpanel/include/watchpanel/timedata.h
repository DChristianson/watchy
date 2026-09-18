#ifndef WATCHPANEL_TIMEDATA_H_
#define WATCHPANEL_TIMEDATA_H_

#include "data.h"

namespace watchpanel {

    class TimeData : public DataImport {
    public:

        TimeData();
        ~TimeData();
    
        void Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds);

    };

}

#endif // WATCHPANEL_TIMEDATA_H_