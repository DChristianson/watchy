#ifndef HAMPER_H_
#define HAMPER_H_

#include "rapidjson/document.h"

namespace hamper {

    int fetch_url(const char *url, rapidjson::Document &document, const char *target_file_name);

}

#endif // HAMPER_H_