#ifndef WATCHPANEL_MODEL_H_
#define WATCHPANEL_MODEL_H_

#include "rapidjson/document.h"

#include <string>

namespace watchpanel {

    class Model {
    public:

        virtual ~Model();

        virtual bool Find(const char *varName, std::string &result) const;

        // Number of elements at `path` if it points at a JSON array, else 0.
        // Lets a component (e.g. FlipGraphic) discover how many items it has
        // to cycle through without depending on rapidjson directly.
        virtual int GetArraySize(const char *path) const;

    };

    class DocumentModel : public Model {
    private:

        const rapidjson::Document &doc;

    public:

        DocumentModel(const rapidjson::Document &doc);
        ~DocumentModel();

        bool Find(const char *varName, std::string &result) const;
        int GetArraySize(const char *path) const;

    };

    // Wraps another Model so that a relative varName (one that doesn't
    // start with '/') resolves against `basePath` instead of the
    // document root -- e.g. with basePath "/news/items/2", a relative
    // "title" resolves as "/news/items/2/title". Absolute varNames
    // (leading '/') pass straight through to the base model unchanged,
    // so a page can still reach outside the current scope (e.g. "/time/hh")
    // from within it.
    class ScopedModel : public Model {
    private:

        const Model &base;
        std::string basePath;

    public:

        ScopedModel(const Model &base, const std::string &basePath);
        ~ScopedModel();

        bool Find(const char *varName, std::string &result) const;
        int GetArraySize(const char *path) const;

    };

}

#endif // WATCHPANEL_MODEL_H_
