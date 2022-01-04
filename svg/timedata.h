#ifndef WATCHPANEL_TIMEDATA_H_
#define WATCHPANEL_TIMEDATA_H_

#include "data.h"

namespace watchpanel {

    class TimeData : public DataImport {
    public:

        TimeData();
        ~TimeData();
    
        void Pull(const Model &model, rapidjson::Document &out);

    };

}

#endif // WATCHPANEL_TIMEDATA_H_