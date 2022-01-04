#ifndef WATCHPANEL_MODEL_H_
#define WATCHPANEL_MODEL_H_

#include "rapidjson/document.h"

namespace watchpanel {

    class Model {
    public:

        virtual ~Model();

        virtual bool Find(const char *varName, std::string &result) const;

    };

    class DocumentModel : public Model {
    private:

        const rapidjson::Document &doc;

    public:

        DocumentModel(const rapidjson::Document &doc);
        ~DocumentModel();

        bool Find(const char *varName, std::string &result) const;

    };

}

#endif // WATCHPANEL_MODEL_H_